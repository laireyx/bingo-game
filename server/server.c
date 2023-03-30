#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "constant.h"
#include "error.h"
#include "server.h"

static int _poll(bingo_server server);
static void _accept_new_client(bingo_server server);
static void _handle_client(bingo_server server, int clnt_sock);

bingo_server server_init(unsigned int addr, unsigned short port) {
  struct sockaddr_in serv_adr;
  bingo_server server = malloc(sizeof(struct __bingo_server_struct));

  server->socket_fd = socket(PF_INET, SOCK_STREAM, 0);

  memset(&serv_adr, 0, sizeof(serv_adr));
  serv_adr.sin_family = AF_INET;
  serv_adr.sin_addr.s_addr = htonl(addr);
  serv_adr.sin_port = htons(port);

  if (bind(server->socket_fd, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) ==
      -1) {
    error_handling("bind() error");
    return 0;
  }
  if (listen(server->socket_fd, 5) == -1) {
    error_handling("listen() error");
    return 0;
  }

  // Initialize select() multiplexer

  FD_ZERO(&server->reads);
  FD_SET(server->socket_fd, &server->reads);

  return server;
}

int server_run(bingo_server server) {
  int fd;

  while (_poll(server) > 0) {
    for (fd = 0; fd <= server->fd_max; fd++) {
      if (FD_ISSET(fd, &server->reads)) {
        if (fd == server->socket_fd) {
          _accept_new_client(server);
        } else {
          _handle_client(server, fd);
        }
      }
    }
  }

  return 0;
}

int server_close(bingo_server server) {
  if (close(server->socket_fd) < 0) {
    return -1;
  }

  free(server);

  return 0;
}

static int _poll(bingo_server server) {
  int fd_num;
  struct timeval timeout;

  timeout.tv_sec = 5;
  timeout.tv_usec = 5000;

  while (1) {
    if ((fd_num = select(server->fd_max + 1, &server->reads, 0, 0, &timeout)) ==
        -1)
      return -1;
    if (fd_num == 0) continue;

    return fd_num;
  }
}

static void _accept_new_client(bingo_server server) {
  int clnt_fd;
  struct sockaddr_in clnt_adr;
  in_addr_t adr_sz;

  adr_sz = sizeof(clnt_adr);
  clnt_fd = accept(server->socket_fd, (struct sockaddr *)&clnt_adr, &adr_sz);
  FD_SET(clnt_fd, &server->reads);

  server->connection_count++;
  if (server->fd_max < clnt_fd) server->fd_max = clnt_fd;

  printf("connected client: %d\n", clnt_fd);
}

static void _handle_client(bingo_server server, int clnt_fd) {
  int str_len;
  char buf[BUF_SIZE];

  str_len = read(clnt_fd, buf, BUF_SIZE);

  if (str_len == 0) {
    FD_CLR(clnt_fd, &server->reads);
    close(clnt_fd);

    server->connection_count--;
    printf("closed client: %d\n", clnt_fd);
  } else {
    write(clnt_fd, "Hello\n", 7);
  }
}