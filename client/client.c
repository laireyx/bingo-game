#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "constant.h"
#include "message.h"
#include "error.h"
#include "client.h"

static int _read_board_file(bingo_client client, char *file_name);
static void _update_board(bingo_client client, bingo_message_s2c *message);
static unsigned char _choose_bingo_number(bingo_client client);
static unsigned char _get_bingo_count(bingo_client client);
static void _respond(bingo_client client);
static void _print_bingo_board(bingo_client client);

bingo_client client_init(char *addr, char *file_name, unsigned short port) {
  struct sockaddr_in serv_adr;
  bingo_client client = malloc(sizeof(struct __bingo_client_struct));

  client->socket_fd = socket(PF_INET, SOCK_STREAM, 0);

  if (client->socket_fd < 0) {
    error_handling("socket() error");
    free(client);
    return 0;
  }

  memset(&serv_adr, 0, sizeof(serv_adr));
  serv_adr.sin_family = AF_INET;
  serv_adr.sin_addr.s_addr = inet_addr(addr);
  serv_adr.sin_port = htons(port);

  if (connect(client->socket_fd, (struct sockaddr *)&serv_adr,
              sizeof(serv_adr)) == -1) {
    error_handling("connect() error");
    free(client);
    return 0;
  }

  if (_read_board_file(client, file_name) < 0) {
    error_handling("read board file error");
    free(client);
    return 0;
  }

  return client;
}

int client_run(bingo_client client) {
  int i, j;

  bingo_message_s2c message;
  bingo_message_c2s clnt_msg;

  while (read_s2c(client->socket_fd, &message) == 0) {
    printf("[server] bingo_number=%d turn=%d\n", message.bingo_number,
           message.your_turn);

    if (message.game_finished) {
      puts(message.msg);
      return -1;
    }

    _update_board(client, &message);
    _respond(client);
    _print_bingo_board(client);
  }

  return 0;
}

int client_close(bingo_client client) {
  if (close(client->socket_fd) < 0) {
    return -1;
  }

  free(client);

  return 0;
}

static int _read_board_file(bingo_client client, char *file_name) {
  int i, j;
  int bingo_number;
  FILE *board_file = fopen(file_name, "r");

  if (board_file == 0) {
    return -1;
  }

  for (i = 0; i < 5; i++) {
    for (j = 0; j < 5; j++) {
      fscanf(board_file, "%d", &bingo_number);
      client->bingo_board[i][j] = (unsigned char)bingo_number;
      client->checked[i][j] = 0;
    }
  }

  fclose(board_file);

  return 0;
}

static void _update_board(bingo_client client, bingo_message_s2c *message) {
  int i, j;
  unsigned char bingo_number = message->bingo_number;

  for (i = 0; i < 5; i++) {
    for (j = 0; j < 5; j++) {
      if (client->bingo_board[i][j] == message->bingo_number) {
        client->checked[i][j] = 1;
        break;
      }
    }
  }
}

static unsigned char _choose_bingo_number(bingo_client client) {
  int i, j;

  for (i = 0; i < 5; i++) {
    for (j = 0; j < 5; j++) {
      if (!client->checked[i][j]) {
        return client->bingo_board[i][j];
      }
    }
  }

  return 0xff;
}

static unsigned char _get_bingo_count(bingo_client client) {
  int i, j;

  unsigned char bingo_count = 0;

  unsigned char row_count = 0, column_count = 0;
  unsigned char topleft_count = 0, topright_count = 0;

  // 가로
  for (i = 0; i < 5; i++) {
    row_count = column_count = 0;
    for (j = 0; j < 5; j++) {
      row_count += client->checked[i][j];
      column_count += client->checked[j][i];
    }

    if (row_count == 5) bingo_count++;
    if (column_count == 5) bingo_count++;
  }

  for (i = 0; i < 5; i++) {
    topleft_count += client->checked[i][i];
    topright_count += client->checked[i][4 - i];
  }

  if (topleft_count == 5) bingo_count++;
  if (topright_count == 5) bingo_count++;

  return bingo_count;
}

static void _respond(bingo_client client) {
  bingo_message_c2s message;

  message.bingo_count = _get_bingo_count(client);
  message.bingo_number = _choose_bingo_number(client);

  write_c2s(client->socket_fd, &message);
}

static void _print_bingo_board(bingo_client client) {
  int i, j;

  printf("Current bingo count = %d\n", _get_bingo_count(client));
  for (i = 0; i < 5; i++) {
    for (j = 0; j < 5; j++) {
      printf("%d ", client->checked[i][j]);
    }
    printf("\n");
  }
}
