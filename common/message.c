#include <unistd.h>
#include <string.h>
#include "./message.h"

int read_c2s(int fd, bingo_message_c2s *message) {
  unsigned char buf[sizeof(bingo_message_c2s)];

  if (read(fd, buf, sizeof(bingo_message_c2s)) < sizeof(bingo_message_c2s)) {
    return -1;
  }

  message->bingo_count = buf[0];
  message->bingo_number = buf[1];
  return 0;
}

int write_s2c(int fd, bingo_message_s2c *message) {
  unsigned char buf[sizeof(bingo_message_s2c)];

  buf[0] = message->game_finished;
  buf[1] = message->your_turn;
  buf[2] = message->bingo_number;
  strcpy(buf + 3, message->msg);

  if (write(fd, buf, sizeof(bingo_message_s2c)) < sizeof(bingo_message_s2c)) {
    return -1;
  }

  return 0;
}

int read_s2c(int fd, bingo_message_s2c *message) {
  unsigned char buf[sizeof(bingo_message_s2c)];

  if (read(fd, buf, sizeof(bingo_message_s2c)) < sizeof(bingo_message_s2c)) {
    return -1;
  }

  message->game_finished = buf[0];
  message->your_turn = buf[1];
  message->bingo_number = buf[2];
  strcpy(message->msg, buf + 3);
  return 0;
}
int write_c2s(int fd, bingo_message_c2s *message) {
  unsigned char buf[sizeof(bingo_message_c2s)];

  buf[0] = message->bingo_count;
  buf[1] = message->bingo_number;

  if (write(fd, buf, sizeof(bingo_message_c2s)) < sizeof(bingo_message_c2s)) {
    return -1;
  }

  return 0;
}
