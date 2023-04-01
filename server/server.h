#include <sys/select.h>
#include <sys/time.h>

typedef struct __bingo_server_struct {
  /** @brief server socket file descriptor number. */
  int socket_fd;

  /** @brief fd_set struct for select(). */
  fd_set reads;
  /** @brief fd_set result from select(). */
  fd_set select_result;
  /** @brief select() timeout. */
  struct timeval timeout;

  /** @brief Bound for looping select() result. */
  int fd_max;
  /** @brief Number of currently connected clients. */
  int connection_count;
}* bingo_server;

/**
 * @brief Create a new bingo server.
 * @details Initializes server socket, bind() and listen().
 * After successful server socket intialization, prepares fd_set for
 * multiplexing.
 *
 * @returns positive pointer of bingo server instance if success.
 */
bingo_server server_init(unsigned int addr, unsigned short port);

/**
 * @brief Start a bingo server.
 * @details Polling opened file descriptors, then accept() new clients or
 * properly handles with connected clients.
 *
 * @returns zero if graceful exit.
 */
int server_run(bingo_server server);

/**
 * @brief Close a bingo server.
 * @details close() server socket, and free server struct.
 *
 * @returns zero if success.
 */
int server_close(bingo_server server);
