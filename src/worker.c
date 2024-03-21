#include <ft_nmap.h>

static void* NMAP_workerMain(void* arg) {
  printf("Launching one scan\n");
  NMAP_WorkerOptions* const options = arg;
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
  for (uint64_t idx = 0; idx < options->nPorts; ++idx)
    close(sockets[idx]);
  return result;
}

int NMAP_spawnWorkers(const NMAP_Options* options) {
  NMAP_WorkerData* const workers = malloc(sizeof(NMAP_WorkerData) * options->speedup);

  if (!workers) {
    perror("malloc");
    return NMAP_FAILURE;
  }

  uint16_t maxPortsPerWorker = options->nPorts / options->speedup;
  uint8_t nThreads = 0;
  uint16_t portsLeft = options->nPorts;
  uint16_t remainder = options->nPorts % options->speedup;

  for (uint8_t i = 0; portsLeft && i < options->speedup; ++i) {
    ++nThreads;
    workers[i].options.ip = options->ip;
    workers[i].options.scan = options->scan;
    workers[i].options.nPorts = maxPortsPerWorker;
    if (remainder) {
      workers[i].options.nPorts += 1;
      --remainder;
    }
    if (portsLeft < maxPortsPerWorker)
      workers[i].options.nPorts = portsLeft;
    for (uint16_t j = 0; j < workers[i].options.nPorts; ++j)
      workers[i].options.ports[j] = options->ports[options->nPorts - portsLeft--];
  }
  for (uint8_t i = 0; i < nThreads; ++i)
    if (pthread_create(&workers[i].thread, NULL, NMAP_workerMain, (void*)&workers[i].options)) {
      for (size_t j = 0; j < i; ++j)
        pthread_cancel(workers[j].thread);
      for (size_t j = 0; j < i; ++j)
        pthread_join(workers[j].thread, NULL);
      perror("ft_nmap: failed to spawn a thread");
      free(workers);
      return NMAP_FAILURE;
    }

  bool threadError = false;

  for (size_t i = 0; !threadError && i < nThreads; ++i) {
    if (pthread_join(workers[i].thread, &workers[i].result)) {
      threadError = true;
      perror("ft_nmap: failed to join a thread");
    }
    else if (!workers[i].result) {
      threadError = true;
      fputs("ft_nmap: an error occured in a worker thread\n", stderr);
    }
  }
  if (threadError) {
    for (size_t i = 0; i < nThreads; ++i)
      free(workers[i].result);
    free(workers);
    return NMAP_FAILURE;
  }
  for (size_t i = 0; i < nThreads; ++i) {
    // do something with results
    const NMAP_PortStatus* result = workers[i].result;
    for (uint64_t j = 0; j < options->nPorts; ++j) {
      printf("port %d status = %s\n", options->ports[j], result[j] == OPEN ? "open" : "close");
    }
    free(workers[i].result);
  }
  free(workers);
  return NMAP_SUCCESS;
}
