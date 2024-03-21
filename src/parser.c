#include <ft_nmap.h>

static in_addr_t dnsLookup(const char* name) {
  const struct addrinfo hints = {
    .ai_family = AF_INET,
  };
  struct addrinfo* result;

  if (getaddrinfo(name, NULL, &hints, &result))
    return INADDR_NONE;

  const in_addr_t ip = ((struct sockaddr_in*)result->ai_addr)->sin_addr.s_addr;
  freeaddrinfo(result);
  return ip;
}

static error_t parseOpt(int key, char* arg, struct argp_state* state) {
  static bool duplicatePort = false;
  NMAP_Options* const input = state->input;
  char* endptr;

  switch (key) {
  case NMAP_KEY_IP:
    if ((input->ip = dnsLookup(arg)) == INADDR_NONE)
      argp_failure(state, 1, errno, "Failed to resolve hostname '%s'", arg);
    break;

  case NMAP_KEY_SCAN:
    input->scan = NMAP_getScanNumber(arg);
    if (input->scan == NMAP_SCAN_INVALID)
      argp_error(state, "Invalid scan type '%s'", arg);
    break;

  case NMAP_KEY_FILE:
    char* name = NULL;
    size_t nameLength = 0;
    FILE* const file = fopen(arg, "r");

    if (!file)
      argp_failure(state, 1, errno, "Failed to open file '%s'", arg);

    const ssize_t readRet = getline(&name, &nameLength, file);

    fclose(file);
    if (readRet == -1) {
      free(name);
      argp_failure(state, 1, errno, "Failed to read from file '%s'", arg);
    }
    if (readRet && name[readRet - 1] == '\n')
      name[readRet - 1] = '\0';
    input->ip = dnsLookup(name);
    if (input->ip == INADDR_NONE) {
      fprintf(stderr, "ft_nmap: Failed to resolve hostname '%s'\n", name);
      free(name);
      return NMAP_FAILURE;
    }
    free(name);
    break;

  case NMAP_KEY_SPEEDUP:
    unsigned long speedup = strtoul(arg, (char**)&endptr, 0);
    if (errno == ERANGE || *endptr || speedup < 1 || speedup > 250)
      argp_error(state, "Invalid speedup value '%s' (should be an integer in the range [1, 250])", arg);
    input->speedup = speedup;
    break;

  case NMAP_KEY_PORTS:
    if (!*arg)
      argp_error(state, "Invalid argument for --ports: ''");

    const char* cursor = arg;

    while (*cursor) {
      unsigned long begin = strtoul(cursor, &endptr, 0);
      if (errno == ERANGE || begin > UINT16_MAX)
        argp_error(state, "Invalid argument for --ports: '%s'", arg);
      if (*endptr == '-') {
        unsigned long end = strtoul(endptr + 1, &endptr, 0);
        if (errno == ERANGE || end < begin || end > UINT16_MAX || (*endptr && *endptr != ','))
          argp_error(state, "Invalid argument for --ports: '%s'", arg);
        while (begin <= end) {
          bool duplicate = false;
          for (size_t i = 0; i < input->nPorts; i++) {
            if (input->ports[i] == begin) {
              duplicate = duplicatePort = true;
              break;
            }
          }
          if (!duplicate)
            input->ports[input->nPorts++] = begin;
          ++begin;
        }
      }
      else if (*endptr && *endptr != ',')
        argp_error(state, "Invalid argument for --ports: '%s'", arg);
      cursor = endptr + !!*endptr;
    }
    break;

  case ARGP_KEY_END:
    if (input->ip == INADDR_NONE)
      argp_error(state, "No destination found, either --file or --ip must be provided");
    if (duplicatePort)
      fputs("WARNING: Duplicate port number(s) specified.  Are you alert enough to be using Nmap?  Have some coffee "
            "or Jolt(tm).\n",
            stderr);
    if (!input->nPorts) {
      input->nPorts = UINT16_MAX;
      for (uint16_t i = 0; i < UINT16_MAX; i++)
        input->ports[i] = i;
    }
    break;

  default:
    return ARGP_ERR_UNKNOWN;
  }
  return NMAP_SUCCESS;
}

NMAP_Options NMAP_parseArgs(int argc, char** argv) {
  NMAP_Options options = {.ip = INADDR_NONE, .scan = NMAP_SCAN_SYN, .speedup = 1};
  static const struct argp_option argOptions[] = {
    {.name = "ip", .key = NMAP_KEY_IP, .arg = "IP_OR_DOMAIN_NAME", .doc = "The IP address to scan"},
    {.name = "file", .key = NMAP_KEY_FILE, .arg = "PATH", .doc = "The file containing the IP address to scan"},
    {.name = "scan", .key = NMAP_KEY_SCAN, .arg = "SYN|NULL|ACK|FIN|XMAS|UDP", .doc = "The type of scan to perform"},
    {.name = "speedup", .key = NMAP_KEY_SPEEDUP, .arg = "THREADS", .doc = "The number of threads to use"},
    {.name = "ports", .key = NMAP_KEY_PORTS, .arg = "PORTS", .doc = "The ports to scan (eg: 1-10 or 1,2,3 or 1,5-15)"},
    {},
  };
  static const struct argp argp = {.options = argOptions, .parser = parseOpt, .doc = "Nmap, but worse."};

  if (argp_parse(&argp, argc, argv, ARGP_NO_ARGS, NULL, &options) == NMAP_FAILURE)
    exit(EXIT_FAILURE);
  return options;
}
