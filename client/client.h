#include <sys/select.h>
#include <sys/time.h>

typedef struct __bingo_client_struct {
  // server sock
  int socket_fd;

  unsigned char bingo_board[5][5];
  unsigned char checked[5][5];

  unsigned char prev_bingo_number;

} *bingo_client;

bingo_client client_init(char *addr, char *file_name, unsigned short port);

int client_run(bingo_client client);

int client_close(bingo_client client);