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
static void _handle_client(bingo_server server, int clnt_sock);

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
    bingo_message_s2c serv_msg;
    serv_msg.your_turn = 1;
    serv_msg.game_finished = 0;
    serv_msg.bingo_number = 0;
    write_s2c(server->clnt_fd[0], &serv_msg);
    serv_msg.your_turn = 0;
    write_s2c(server->clnt_fd[1], &serv_msg);
  }
}

static void _handle_client(bingo_server server, int clnt_fd) {
  bingo_message_c2s clnt_msg;
  bingo_message_s2c serv_msg;

  int ano_clnt_fd =
      (clnt_fd == server->clnt_fd[0] ? server->clnt_fd[1] : server->clnt_fd[0]);

  if (read_c2s(clnt_fd, &clnt_msg) < 0) {
    FD_CLR(clnt_fd, &server->reads);
    close(clnt_fd);

    server->connection_count--;
    printf("closed client: %d\n", clnt_fd);
  } else {
    if (clnt_msg.bingo_count >= 3) {
      printf(">= 3 bingo: game finished\n");
      serv_msg.game_finished = 1;

      strcpy(serv_msg.msg, "YOU WIN");
      write_s2c(clnt_fd, &serv_msg);
      strcpy(serv_msg.msg, "YOU LOSE");
      write_s2c(ano_clnt_fd, &serv_msg);

      return;
    } else {
      serv_msg.bingo_number = clnt_msg.bingo_number;
      serv_msg.your_turn = 2;

      if (clnt_msg.bingo_number != 77) {
        printf("client said '%d'\n", clnt_msg.bingo_number);
        // 두 클라이언트 모두에게 현재 숫자를 공지
        write_s2c(clnt_fd, &serv_msg);
        write_s2c(ano_clnt_fd, &serv_msg);
        // 공지가 끝났으면 다음 클라이언트에게 차례 보냄
        serv_msg.game_finished = 0;
        serv_msg.your_turn = 1;
        write_s2c(ano_clnt_fd, &serv_msg);
      }
    }
  }
}
