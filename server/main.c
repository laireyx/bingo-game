#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "server.h"
#include "error.h"

int main(int argc, char** argv) {
  int clnt_sock;
  socklen_t adr_sz;

  bingo_server server;

  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    return 1;
  }

  server = server_init(INADDR_ANY, atoi(argv[1]));
  if (server < 0) {
    error_handling("server error");
    return 1;
  }

  server_run(server);

  return 0;
}
