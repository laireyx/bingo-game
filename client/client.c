#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "message.h"
#include "constant.h"
#include "client.h"

bingo_client client_init(char *addr, char *file_name, unsigned short port) {
  bingo_client client = malloc(sizeof(struct __bingo_client_struct));

  int sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock == -1) return 0;

  struct sockaddr_in serv_adr;
  memset(&serv_adr, 0, sizeof(serv_adr));
  serv_adr.sin_family = AF_INET;
  serv_adr.sin_addr.s_addr = inet_addr(addr);
  serv_adr.sin_port = htons(port);

  if (connect(sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
    return 0;

  client->sock = sock;

  FILE *file = fopen(file_name, "r");
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      int temp;
      fscanf(file, "%d", &temp);
      client->bingo_board[i][j] = (unsigned char)temp;
      client->checked[i][j] = 0;
    }
  }

  return client;
}

unsigned char get_bingo_count(bingo_client client) {
  unsigned char bingo_count = 0;
  // 가로
  for (int i = 0; i < 5; i++) {
    unsigned char cnt1 = 0, cnt2 = 0;
    for (int j = 0; j < 5; j++) {
      cnt1 += client->checked[i][j];
      cnt2 += client->checked[j][i];
    }
    if (cnt1 == 5) bingo_count++;
    if (cnt2 == 5) bingo_count++;
  }

  unsigned char cnt1 = 0, cnt2 = 0;
  for (int i = 0; i < 5; i++) {
    cnt1 += client->checked[i][i];
    cnt2 += client->checked[i][4 - i];
  }
  if (cnt1 == 5) bingo_count++;
  if (cnt2 == 5) bingo_count++;

  return bingo_count;
}

void print_bingo_board(bingo_client client) {
  printf("Current bingo count = %d\n", get_bingo_count(client));
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      printf("%d ", client->checked[i][j]);
    }
    printf("\n");
  }
}

int client_run(bingo_client client) {
  char buf[BUF_SIZE];

  bingo_message_s2c serv_msg;
  bingo_message_c2s clnt_msg;
  while (1) {
    read_s2c(client->sock, &serv_msg);
    printf("[server] bingo_number=%d turn=%d\n", serv_msg.bingo_number,
           serv_msg.your_turn);
    if (serv_msg.game_finished) {
      puts(serv_msg.msg);
      break;
    } else if (serv_msg.your_turn == 1) {
      int flag = 0;
      for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
          if (!client->checked[i][j]) {
            client->checked[i][j] = 1;
            clnt_msg.bingo_number = client->bingo_board[i][j];
            flag = 1;
            break;
          }
        }
        if (flag) break;
      }
      clnt_msg.bingo_count = get_bingo_count(client);
      printf("Ok. my turn. I say '%d'\n", clnt_msg.bingo_number);
      client->prev_bingo_number = clnt_msg.bingo_number;
      write_c2s(client->sock, &clnt_msg);
    } else if (serv_msg.your_turn == 2) {
      unsigned char bingo_number = serv_msg.bingo_number;
      for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
          if (client->bingo_board[i][j] == bingo_number) {
            if (client->prev_bingo_number != bingo_number) {
              printf("Ok. his turn. I said '%d', He say '%d'\n",
                     client->prev_bingo_number, bingo_number);
            }
            client->checked[i][j] = 1;
            break;
          }
        }
      }
      clnt_msg.bingo_count = get_bingo_count(client);
      clnt_msg.bingo_number = 77;
      write_c2s(client->sock, &clnt_msg);
    }
    print_bingo_board(client);
  }
}