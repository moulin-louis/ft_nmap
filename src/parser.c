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

static int ArrayFn_setUint16ToIndex(unused Array* arr, unused size_t i, void* value, unused void* param) {
  *(uint16_t*)value = i;
  return 0;
}

static error_t parseOpt(int key, char* arg, struct argp_state* state) {
  static bool duplicatePort = false;
  static bool duplicateIp = false;
  NMAP_Options* const input = state->input;
  const char* tok;
  char* cursor = arg;
  char* endptr;
  in_addr_t ip;
  char* name = NULL;
  size_t nameLength = 0;
  FILE* const file = fopen(arg, "r");
  ssize_t readRet;
  uint32_t scan;
  unsigned long speedup = strtoul(arg, (char**)&endptr, 0);

  switch (key) {
  case NMAP_KEY_IP:
    while ((tok = strsep(&cursor, ","))) {
      if (!*tok) {
        if (cursor)
          cursor[-1] = ',';
        argp_error(state, "Invalid argument for --ip: '%s'", arg);
      }
      if ((ip = dnsLookup(tok)) == INADDR_NONE)
        argp_failure(state, 1, errno, "Failed to resolve hostname '%s'", tok);
      if (cursor)
        cursor[-1] = ',';
      if (array_any(input->ips, &ip))
        duplicateIp = true;
      else if (array_pushBack(input->ips, &ip, 1))
        return NMAP_FAILURE;
    }
    break;

  case NMAP_KEY_FILE:
    if (!file)
      argp_failure(state, 1, errno, "Failed to open file '%s'", arg);

    while ((readRet = getline(&name, &nameLength, file)) != -1) {
      if (!readRet)
        continue;
      if (name[readRet - 1] == '\n')
        name[readRet - 1] = 0;
      if ((ip = dnsLookup(name)) == INADDR_NONE) {
        fprintf(stderr, "ft_nmap: Failed to resolve hostname '%s'\n", name);
        free(name);
        return NMAP_FAILURE;
      }
      if (array_any(input->ips, &ip))
        duplicateIp = true;
      else if (array_pushBack(input->ips, &ip, 1)) {
        free(name);
        return NMAP_FAILURE;
      }
    }
    if (errno == ENOMEM)
      argp_failure(state, 1, errno, "Failed to read from file '%s'", arg);

    fclose(file);
    if (readRet == -1) {
      free(name);
      argp_failure(state, 1, errno, "Failed to read from file '%s'", arg);
    }
    if (readRet && name[readRet - 1] == '\n')
      name[readRet - 1] = '\0';
    if ((ip = dnsLookup(name)) == INADDR_NONE) {
      fprintf(stderr, "ft_nmap: Failed to resolve hostname '%s'\n", name);
      free(name);
      return NMAP_FAILURE;
    }
    if (array_pushBack(input->ips, &ip, 1)) {
      free(name);
      return NMAP_FAILURE;
    }
    free(name);
    break;

  case NMAP_KEY_SCAN:
    while ((tok = strsep(&cursor, ","))) {
      scan = NMAP_getScanNumber(tok);
      if (cursor)
        cursor[-1] = ',';
      if (scan == NMAP_SCAN_NONE)
        argp_error(state, "Invalid argument for --scan: '%s'", arg);
      input->scan |= scan;
    }
    break;

  case NMAP_KEY_SPEEDUP:
    if (errno == ERANGE || *endptr || speedup < 1 || speedup > 250)
      argp_error(state, "Invalid speedup value '%s' (should be an integer in the range [1, 250])", arg);
    input->speedup = speedup;
    break;

  case NMAP_KEY_PORTS:
    if (!*arg)
      argp_error(state, "Invalid argument for --ports: ''");

    while (*cursor) {
      unsigned long begin = strtoul(cursor, &endptr, 0);
      if (errno == ERANGE || begin > UINT16_MAX)
        argp_error(state, "Invalid argument for --ports: '%s'", arg);
      if (*endptr == '-') {
        unsigned long end = strtoul(endptr + 1, &endptr, 0);
        if (errno == ERANGE || end < begin || end > UINT16_MAX || (*endptr && *endptr != ','))
          argp_error(state, "Invalid argument for --ports: '%s'", arg);
        for (uint16_t port = begin; port <= end; ++port)
          if (array_any(input->ports, &port))
            duplicatePort = true;
          else if (array_pushBack(input->ports, &port, 1))
            return NMAP_FAILURE;
      }
      else {
        if (*endptr && *endptr != ',')
          argp_error(state, "Invalid argument for --ports: '%s'", arg);
        const uint16_t port = begin;
        if (array_any(input->ports, &port))
          duplicatePort = true;
        else if (array_pushBack(input->ports, &port, 1))
          return NMAP_FAILURE;
      }
      cursor = endptr + !!*endptr;
    }
    break;

  case ARGP_KEY_END:
    if (input->scan == NMAP_SCAN_NONE)
      input->scan = NMAP_SCAN_ALL;
    if (array_empty(input->ips))
      argp_error(state, "No destination found, either --file or --ip must be provided");
    if (duplicateIp)
      fputs("WARNING: Duplicate destination address(es) specified.  Are you alert enough to be using Nmap?  Have some "
            "coffee "
            "or Jolt(tm).\n",
            stderr);
    if (duplicatePort)
      fputs("WARNING: Duplicate port number(s) specified.  Are you alert enough to be using Nmap?  Have some coffee "
            "or Jolt(tm).\n",
            stderr);
    duplicateIp = duplicatePort = false;
    if (array_empty(input->ports)) {
      if (array_resize(input->ports, 1025))
        return NMAP_FAILURE;
      array_forEach(input->ports, ArrayFn_setUint16ToIndex, NULL);
    }
    array_shrink(input->ips);
    array_shrink(input->ports);
    break;

  default:
    return ARGP_ERR_UNKNOWN;
  }
  return NMAP_SUCCESS;
}

static void NMAP_destroyOptions(int status, void* arg) {
  (void)status;
  NMAP_Options* options = arg;

  array_destroy(options->ips);
  array_destroy(options->ports);
  free(options);
}

NMAP_Options* NMAP_parseArgs(int argc, char** argv) {
  NMAP_Options* options = malloc(sizeof(NMAP_Options));
  if (!options)
    return NULL;

  memset(options, 0, sizeof(NMAP_Options));
  options->speedup = 1;
  options->ips = array(sizeof(in_addr_t), 1, 0, NULL, NULL);
  options->ports = array(sizeof(uint16_t), UINT16_MAX + 1, 0, NULL, NULL);

  on_exit(NMAP_destroyOptions, options);
  if (!options->ips || !options->ports)
    return NULL;

  static const struct argp_option argOptions[] = {
    {.name = "ip", .key = NMAP_KEY_IP, .arg = "IP_OR_DOMAIN_NAME", .doc = "The IP address to scan"},
    {.name = "file", .key = NMAP_KEY_FILE, .arg = "PATH", .doc = "The file containing the IP address to scan"},
    {.name = "scan", .key = NMAP_KEY_SCAN, .arg = "SYN|NULL|ACK|FIN|XMAS|UDP", .doc = "The type of scan to perform"},
    {.name = "speedup", .key = NMAP_KEY_SPEEDUP, .arg = "THREADS", .doc = "The number of threads to use"},
    {.name = "ports", .key = NMAP_KEY_PORTS, .arg = "PORTS", .doc = "The ports to scan (eg: 1-10 or 1,2,3 or 1,5-15)"},
    {},
  };
  static const struct argp argp = {.options = argOptions, .parser = parseOpt, .doc = "Nmap, but worse."};

  if (argp_parse(&argp, argc, argv, ARGP_NO_ARGS, NULL, options) == NMAP_FAILURE)
    return NULL;
  return options;
}
