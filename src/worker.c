#include <ft_nmap.h>

static void* NMAP_workerMain(void* arg) {
  NMAP_WorkerOptions* const options = arg;
  options->nPorts = 1024;
  printf("Launching the scan of %u ports\n", options->nPorts);
  int32_t sockets[options->nPorts];
  NMAP_PortStatus* result = NULL;

  memset(sockets, 0, options->nPorts * sizeof(int32_t));
  result = calloc(options->nPorts, sizeof(NMAP_PortStatus));
  if (result == NULL)
    return NULL;
  switch (options->scan) {
  case NMAP_SCAN_SYN: {
    if (tcp_syn_init(options->nPorts, sockets))
      return NULL;
    if (tcp_syn_perform(options, sockets, result)) {
      return NULL;
    }
    break;
  }
    default: {
  }
  }
  return result;
}

int NMAP_spawnWorkers(const NMAP_Options* options) {
  printf("launching with %d thread\n", options->speedup);
  pthread_t workers[options->speedup];
  NMAP_WorkerOptions workerOptions[options->speedup];
  void* workerResults[options->speedup];
  uint16_t maxPortsPerWorker = options->nPorts / options->speedup;
  uint8_t nThreads = 0;
  uint16_t portsLeft = options->nPorts;

  if (!maxPortsPerWorker)
    maxPortsPerWorker = 1;
  for (uint8_t i = 0; portsLeft && i < options->speedup; ++i) {
    ++nThreads;
    workerOptions[i].ip = options->ip;
    workerOptions[i].scan = options->scan;
    workerOptions[i].nPorts = maxPortsPerWorker;
    if (portsLeft < maxPortsPerWorker)
      workerOptions[i].nPorts = portsLeft;
    for (uint16_t j = 0; j < workerOptions[i].nPorts; ++j)
      workerOptions[i].ports[j] = options->ports[workerOptions[i].nPorts - portsLeft--];
  }
  for (uint8_t i = 0; i < nThreads; ++i)
    if (pthread_create(&workers[i], NULL, NMAP_workerMain, (void*)&workerOptions[i])) {
      for (size_t j = 0; j < i; ++j)
        pthread_cancel(workers[j]);
      for (size_t j = 0; j < i; ++j)
        pthread_join(workers[j], NULL);
      perror("ft_nmap: failed to spawn a thread");
      return NMAP_FAILURE;
    }
  for (size_t i = 0; i < nThreads; ++i) {
    if (pthread_join(workers[i], &workerResults[i]))
      perror("ft_nmap: failed to join a thread");
    if (!workerResults[i])
      fputs("ft_nmap: an error occured in a worker thread\n", stderr);
  }
  for (size_t i = 0; i < nThreads; ++i) {
    // do something with results
    NMAP_PortStatus* result = workerResults[i];
    printf("port 22 status = %s\n", result[22] == OPEN ? "open" : "close");
    printf("port 80 status = %s\n", result[80] == OPEN ? "open" : "close");
    free(workerResults[i]);
  }
  return NMAP_SUCCESS;
}
