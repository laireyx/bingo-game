#include <unistd.h>
#include "./message.h"

int read_c2s(int fd, bingo_message_c2s* message) {
  unsigned char buf[sizeof(bingo_message_c2s)];

  if (read(fd, buf, sizeof(bingo_message_c2s)) < sizeof(bingo_message_c2s)) {
    return -1;
  }

  message->bingo_count = buf[0];
  return 0;
}
int write_s2c(int fd, bingo_message_s2c* message) {
  unsigned char buf[sizeof(bingo_message_c2s)];

  buf[0] = message->game_finished;

  if (write(fd, buf, sizeof(bingo_message_c2s)) < sizeof(bingo_message_c2s)) {
    return -1;
  }

  return 0;
}

int read_s2c(int fd, bingo_message_s2c* message) {
  unsigned char buf[sizeof(bingo_message_c2s)];

  if (read(fd, buf, sizeof(bingo_message_c2s)) < sizeof(bingo_message_c2s)) {
    return -1;
  }

  message->game_finished = buf[0];
  return 0;
}
int write_c2s(int fd, bingo_message_c2s* message) {
  unsigned char buf[sizeof(bingo_message_c2s)];

  buf[0] = message->bingo_count;

  if (write(fd, buf, sizeof(bingo_message_c2s)) < sizeof(bingo_message_c2s)) {
    return -1;
  }

  return 0;
}
