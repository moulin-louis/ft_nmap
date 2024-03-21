#include <ft_nmap.h>

static void* NMAP_workerMain(void* arg) {
  const NMAP_WorkerOptions* const options = arg;
  int sockets[options->nPorts];

  (void)sockets;
  printf("Hello world!\n");
  return NULL;
}

int NMAP_spawnWorkers(const NMAP_Options* options) {
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
    free(workerResults[i]);
  }
  return NMAP_SUCCESS;
}
