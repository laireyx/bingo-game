#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "constant.h"
#include "message.h"
#include "error.h"
#include "server.h"

static int _poll(bingo_server server);
static void _accept_new_client(bingo_server server);
static void _start_game(bingo_server server);
static void _handle_client(bingo_server server, int clnt_sock);
static void _finish_game(bingo_server server, int winner_fd);
static void _broadcast_game(bingo_server server, unsigned char bingo_number);

bingo_server server_init(unsigned int addr, unsigned short port) {
  int optval = 1;
  struct sockaddr_in serv_adr;
  bingo_server server = malloc(sizeof(struct __bingo_server_struct));

  server->socket_fd = socket(PF_INET, SOCK_STREAM, 0);

  if (server->socket_fd < 0) {
    error_handling("socket() error");
    free(server);
    return 0;
  }

  memset(&serv_adr, 0, sizeof(serv_adr));
  serv_adr.sin_family = AF_INET;
  serv_adr.sin_addr.s_addr = htonl(addr);
  serv_adr.sin_port = htons(port);

  setsockopt(server->socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval,
             sizeof(optval));
  if (bind(server->socket_fd, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) ==
      -1) {
    error_handling("bind() error");
    free(server);
    return 0;
  }
  if (listen(server->socket_fd, 5) == -1) {
    error_handling("listen() error");
    free(server);
    return 0;
  }

  // Initialize select() multiplexer

  FD_ZERO(&server->reads);
  FD_SET(server->socket_fd, &server->reads);

  server->fd_max = server->socket_fd;

  return server;
}

int server_run(bingo_server server) {
  int fd;

  while (_poll(server) > 0) {
    for (fd = 0; fd <= server->fd_max; fd++) {
      if (FD_ISSET(fd, &server->select_result)) {
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
  int fd_num = 0;
  struct timeval timeout;

  timeout.tv_sec = 5;
  timeout.tv_usec = 5000;

  while (1) {
    server->select_result = server->reads;

    if ((fd_num = select(server->fd_max + 1, &server->select_result, 0, 0,
                         &timeout)) == -1)
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

  server->clnt_fd[server->connection_count++] = clnt_fd;
  if (server->fd_max < clnt_fd) server->fd_max = clnt_fd;

  printf("connected client: %d\n", clnt_fd);

  // game start
  if (server->connection_count == 2) {
    _start_game(server);
  }
}

static void _start_game(bingo_server server) {
  bingo_message_s2c serv_msg;

  // Initialize server
  server->turn_fd = server->clnt_fd[0];

  // Notify to the first client
  serv_msg.game_finished = 0;
  serv_msg.your_turn = 1;
  serv_msg.bingo_number = 0;

  write_s2c(server->clnt_fd[0], &serv_msg);
}

static void _handle_client(bingo_server server, int clnt_fd) {
  bingo_message_c2s clnt_msg;

  if (read_c2s(clnt_fd, &clnt_msg) < 0) {
    FD_CLR(clnt_fd, &server->reads);
    close(clnt_fd);

    server->connection_count--;
    printf("closed client: %d\n", clnt_fd);
  } else {
    if (clnt_msg.bingo_count >= 3) {
      _finish_game(server, clnt_fd);
      return;
    }

    if (server->turn_fd == clnt_fd) {
      server->turn_fd = server->clnt_fd[0] + server->clnt_fd[1] - clnt_fd;
      _broadcast_game(server, clnt_msg.bingo_number);
    }
  }
}

static void _finish_game(bingo_server server, int winner_fd) {
  int i;
  bingo_message_s2c message;

  for (i = 0; i < 2; i++) {
    message.game_finished = 1;
    strcpy(message.msg,
           server->clnt_fd[i] == winner_fd ? "YOU WIN" : "YOU LOSE");

    write_s2c(server->clnt_fd[i], &message);
  }
}

static void _broadcast_game(bingo_server server, unsigned char bingo_number) {
  int i;
  bingo_message_s2c message;

  for (i = 0; i < 2; i++) {
    message.game_finished = 0;
    message.your_turn = server->turn_fd == server->clnt_fd[i];
    message.bingo_number = bingo_number;

    write_s2c(server->clnt_fd[i], &message);
  }
}