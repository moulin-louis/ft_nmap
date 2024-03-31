#include <ft_nmap.h>

static bool ArrayFn_hostExist(const Array* arr, size_t i, const void* value, void* param) {
  (void)arr, (void)i, (void)value, (void)param;
  return true;
}

static bool ArrayFn_hostFind(const Array* arr, size_t i, const void* value, void* param) {
  (void)arr, (void)i;
  const t_host* host = value;
  const t_host* host_find = param;
  return host->ip.s_addr == host_find->ip.s_addr;
}


/**
 * merge the result of all scan for an host
 * @param thread_result {Array<Array<t_host>> - An array of array of host, each host contains an array of t_port with
 * their status
 * @return {Array*} - return an array of unique t_host (thread_result can contains multiple time the same t_host)
 */
Array* merge_result(Array* thread_result) {
  // result == Array<t_host>
  Array* result = array(sizeof(t_host), array_size(thread_result), 0, NULL, NULL);
  if (result == NULL) {
    array_destroy(thread_result);
    return NULL;
  }
  for (uint64_t i = 0; i < array_size(thread_result); ++i) {
    // tmp_result == Array<t_host>
    Array* tmp_result = *(Array**)array_get(thread_result, i);
    for (uint64_t j = 0; j < array_size(tmp_result); ++j) {
      t_host* host_tmp = array_get(tmp_result, j);
      if (array_anyIf(result, ArrayFn_hostExist, host_tmp) == false) {
        // Its the first time we see this host_tmp so we simply push it
        array_pushBack(tmp_result, host_tmp, 1);
        continue;
      }
      // We already have a result for this host_tmp so we need to merge both result
      // Find the actual instance of host in thread_result
      t_host* host_result = array_findIf(result, ArrayFn_hostFind, host_tmp);
      // merge result for all ports
      for (uint64_t x = 0; x < array_size(host_tmp->ports); ++x) {
        t_port* port_tmp = array_get(host_tmp->ports, x);
        t_port* port_result = array_get(host_result->ports, x);
        if (port_result->result == OPEN)
          continue;
        if (port_tmp->result == CLOSE)
          port_result->result = CLOSE;
        port_result->result = port_tmp->port;
      }
    }
  }
  return result;
}

static void* NMAP_workerMain(void* arg) {
  printf("Launching one scan\n");
  const NMAP_WorkerOptions* const options = arg;
  Array* thread_result = array(sizeof(Array*), array_size(options->ips), 0, NULL, NULL);
  // thread_result == Array<Array<t_host>>
  if (thread_result == NULL)
    return NULL;
  if (options->scan & NMAP_SCAN_NULL)
    ultra_scan(options->ips, options->ports, NMAP_SCAN_NULL, thread_result);
  if (options->scan & NMAP_SCAN_SYN)
    ultra_scan(options->ips, options->ports, NMAP_SCAN_SYN, thread_result);
  if (options->scan & NMAP_SCAN_ACK)
    ultra_scan(options->ips, options->ports, NMAP_SCAN_ACK, thread_result);
  if (options->scan & NMAP_SCAN_FIN)
    ultra_scan(options->ips, options->ports, NMAP_SCAN_FIN, thread_result);
  if (options->scan & NMAP_SCAN_XMAS)
    ultra_scan(options->ips, options->ports, NMAP_SCAN_XMAS, thread_result);
  // merge result of all scan
  return merge_result(thread_result);
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
