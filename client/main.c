#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "client.h"
#include "error.h"

int main(int argc, char **argv) {
  int clnt_sock;
  socklen_t adr_sz;

  bingo_client client;

  if (argc != 4) {
    printf("Usage: %s <ip_addr> <port> <bingo_file>\n", argv[0]);
    return 1;
  }

  client = client_init(argv[1], argv[3], atoi(argv[2]));
  if (client <= 0) {
    error_handling("server error");
    return 1;
  }

  client_run(client);

  return 0;
}
