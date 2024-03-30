#include <ft_nmap.h>

static void* NMAP_workerMain(void* arg) {
  printf("Launching one scan\n");
  const NMAP_WorkerOptions* const options = arg;

  const size_t nPorts = array_size(options->ports);

  Array* result = array(sizeof(NMAP_PortStatus), nPorts, nPorts, NULL, NULL);
  if (result == NULL)
    return NULL;
  if (options->scan & NMAP_SCAN_NONE) {
    printf("none scan\n");
    ultra_scan(options->ips, options->ports, NMAP_SCAN_NONE);
  }
  if (options->scan & NMAP_SCAN_SYN) {
    printf("syn scan\n");
    ultra_scan(options->ips, options->ports, NMAP_SCAN_SYN);
  }
  if (options->scan & NMAP_SCAN_ACK) {
    printf("ack scan\n");
    ultra_scan(options->ips, options->ports, NMAP_SCAN_ACK);
  }
  if (options->scan & NMAP_SCAN_FIN) {
    printf("fin scan\n");
    ultra_scan(options->ips, options->ports, NMAP_SCAN_FIN);
  }
  if (options->scan & NMAP_SCAN_XMAS) {
    printf("xmas scan\n");
    ultra_scan(options->ips, options->ports, NMAP_SCAN_XMAS);
  }
  return result;
}

static void workerDataDestructor(Array* arr, void* data, size_t n) {
  (void)arr;
  const NMAP_WorkerData* const workers = data;
  for (size_t i = 0; i < n; ++i) {
    array_destroy(workers[i].options.ports);
    array_destroy(workers[i].result);
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

static int ArrayFn_setupWorkerOptions(unused Array* arr, unused size_t i, void* value, void* param) {
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

static int ArrayFn_cancelWorkerThread(unused const Array* arr, unused size_t i, const void* value, unused void* param) {
  const NMAP_WorkerData* const worker = value;

  pthread_cancel(worker->thread);
  return 0;
}

static int ArrayFn_joinWorkerThread(unused Array* arr, unused size_t i, void* value, void* param) {
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

static int ArrayFn_spawnWorkerThread(Array* arr, size_t i, void* value, unused void* param) {
  NMAP_WorkerData* const worker = value;

  if (pthread_create(&worker->thread, NULL, NMAP_workerMain, &worker->options)) {
    array_cForEachWithin(arr, 0, i, ArrayFn_cancelWorkerThread, NULL);
    array_forEachWithin(arr, 0, i, ArrayFn_joinWorkerThread, NULL);
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

  if (array_forEach(workers, ArrayFn_setupWorkerOptions, &setup) ||
      array_forEach(workers, ArrayFn_spawnWorkerThread, NULL))
    return NMAP_FAILURE;

  bool threadError = false;

  array_forEach(workers, ArrayFn_joinWorkerThread, &threadError);

  if (threadError)
    return NMAP_FAILURE;

  return NMAP_SUCCESS;
}
