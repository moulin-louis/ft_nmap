#include <ft_nmap.h>

static void cleanupThreadSockets(void* sockets) { array_destroy(sockets); }

static void socketFdDestructor(Array* arr, void* data, size_t n) {
  (void)arr;
  int* const scks = data;

  for (size_t i = 0; i < n; ++i)
    if (scks[i] > 0)
      close(scks[i]);
}

static int socketsConstructor(Array* arr, void* data, size_t n) {
  (void)arr;
  Array** const sockets = data;

  const ArrayFactory socketElementFactory = {
    .destructor = socketFdDestructor,
  };

  for (size_t i = 0; i < n; ++i) {
    sockets[i] = array(sizeof(int), 0, 0, NULL, &socketElementFactory);

    if (!sockets[i])
      return 1;
  }
  return 0;
}

static void socketsDestructor(Array* arr, void* data, size_t n) {
  (void)arr;
  Array** const sockets = data;

  for (size_t i = 0; i < n; ++i)
    array_destroy(sockets[i]);
}

static void* NMAP_workerMain(void* arg) {
  printf("Launching one scan\n");
  const NMAP_WorkerOptions* const options = arg;
  const ArrayFactory socketsFactory = {
    .constructor = socketsConstructor,
    .destructor = socketsDestructor,
  };

  NMAP_printWorkerOptions(options);

  // Array<Array<int>>
  Array* const sockets = array(sizeof(Array*), 0, 0, NULL, &socketsFactory);

  if (!sockets)
    return NULL;

  NMAP_PortStatus* result = NULL;

  // -----------------
  // Now sockets is a 2D Array containing the sockets for each port for each ip
  // For example to access the socket for the first port of the first ip:
  // array_get(array_get(sockets, 0), 0)
  // or the last port of the last ip:
  // array_get(array_get(sockets, -1), -1)
  // (returns a pointer to the socket)
  // array_cGet is the const version of the function
  // array_get has bounds checking (it will return NULL if the index is out of bounds)
  // if you don't want bounds checking, use array_data or array_cData instead
  // -----------------

  // I added this pthread_cleanup_push call to free automatically the sockets in case
  // of failure, but now you need to call pthread_exit(NULL) instead of just returning
  // NULL, otherwise the cleanup handler won't be called
  pthread_cleanup_push(cleanupThreadSockets, sockets);

  // Now we get the number of ports by looking at the size
  // of the ports array instead of using options->nPorts
  const size_t nPorts = array_size(options->ports);

  // You will now need to use the array above instead
  int32_t sockets[nPorts];

  memset(sockets, 0, nPorts * sizeof(int32_t));
  result = calloc(nPorts, sizeof(NMAP_PortStatus));
  if (result == NULL)
    return NULL;
  if (options->scan & NMAP_SCAN_SYN || true) {
    if (tcp_syn_init(nPorts, sockets))
      return free(result), NULL;
    if (tcp_syn_perform(options, sockets, result))
      return free(result), NULL;
  }
  for (uint64_t idx = 0; idx < nPorts; ++idx)
    close(sockets[idx]);

  if (!sockets)
    return NULL;

  (void)sockets;

  return result;
}

static void workerDataDestructor(Array* arr, void* data, size_t n) {
  (void)arr;
  NMAP_WorkerData* const workers = data;
  for (size_t i = 0; i < n; ++i) {
    array_destroy(workers[i].options.ports);
    free(workers[i].result);
  }
}

static void destroyWorkers(int status, void* arg) {
  (void)status;
  array_destroy(arg);
}

typedef struct s_worker_setup_param {
  const uint16_t minPortsPerWorker;
  const uint16_t scan;
  uint16_t remainder;
  uint16_t portsLeft;
  const Array* const ips;
  const Array* const ports;
} WorkerSetupParam;

static int setupWorkerOptions(Array* arr, size_t i, void* value, void* param) {
  (void)arr, (void)i;
  WorkerSetupParam* const setup = param;
  NMAP_WorkerData* const worker = value;
  const ptrdiff_t from = -setup->portsLeft;
  const ptrdiff_t to = array_size(setup->ports) - setup->portsLeft + setup->minPortsPerWorker + !!setup->remainder;

  worker->options.scan = setup->scan;
  worker->options.ips = setup->ips;
  worker->options.ports = array_sliced(setup->ports, from, to);
  if (!worker->options.ports) {
    perror("ft_nmap");
    return 1;
  }
  setup->portsLeft -= array_size(worker->options.ports);
  if (setup->remainder)
    --setup->remainder;
  return 0;
}

static int cancelWorkerThread(const Array* arr, size_t i, const void* value, void* param) {
  (void)arr, (void)i, (void)param;
  const NMAP_WorkerData* const worker = value;

  pthread_cancel(worker->thread);
  return 0;
}

static int joinWorkerThread(Array* arr, size_t i, void* value, void* param) {
  (void)arr, (void)i;

  bool* const threadError = param;
  NMAP_WorkerData* const worker = value;

  if (pthread_join(worker->thread, &worker->result)) {
    if (threadError)
      *threadError = true;
    perror("ft_nmap: failed to join a thread");
  }
  else if (!worker->result) {
    if (threadError)
      *threadError = true;
    fputs("ft_nmap: an error occured in a worker thread\n", stderr);
  }
  return 0;
}

int spawnWorkerThread(Array* arr, size_t i, void* value, void* param) {
  (void)param;
  NMAP_WorkerData* const worker = value;

  if (pthread_create(&worker->thread, NULL, NMAP_workerMain, &worker->options)) {
    array_cForEachWithin(arr, 0, i, cancelWorkerThread, NULL);
    array_forEachWithin(arr, 0, i, joinWorkerThread, NULL);
    perror("ft_nmap: failed to spawn a thread");
    return 1;
  }
  return 0;
}

int NMAP_spawnWorkers(const NMAP_Options* options) {
  const size_t nPorts = array_size(options->ports);
  const uint16_t nThreads = options->speedup <= nPorts ? options->speedup : nPorts;
  WorkerSetupParam setup = {
    .minPortsPerWorker = nPorts / nThreads,
    .scan = options->scan,
    .remainder = nPorts % nThreads,
    .portsLeft = nPorts,
    .ips = options->ips,
    .ports = options->ports,
  };
  ArrayFactory workersFactory = {
    .destructor = workerDataDestructor,
  };
  Array* const workers = array(sizeof(NMAP_WorkerData), 0, nThreads, NULL, &workersFactory);

  if (!workers) {
    perror("malloc");
    return NMAP_FAILURE;
  }
  on_exit(destroyWorkers, workers);

  if (array_forEach(workers, setupWorkerOptions, &setup) || array_forEach(workers, spawnWorkerThread, NULL))
    return NMAP_FAILURE;

  bool threadError = false;

  array_forEach(workers, joinWorkerThread, &threadError);

  if (threadError)
    return NMAP_FAILURE;
<<<<<<< HEAD
}
for (size_t i = 0; i < nThreads; ++i) {
  // do something with results
  const NMAP_PortStatus* result = workers[i].result;
  for (uint64_t j = 0; j < options->nPorts; ++j) {
    if (result[j] != CLOSE) {
      printf("port %d status = %s\n", options->ports[j], port_status_to_string(result[j]));
    }
  }
  free(workers[i].result);
}
free(workers);
=======

  // do something with results

>>>>>>> lsuardi
return NMAP_SUCCESS;
}
