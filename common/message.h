
#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

typedef struct {
  unsigned char bingo_count;
}
#ifdef __GNUC__
__attribute__((packed))
#endif
bingo_message_c2s;

typedef struct {
  unsigned char game_finished;
}
#ifdef __GNUC__
__attribute__((packed))
#endif
bingo_message_s2c;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

/**
 * @brief Read client-to-server message from the socket.
 * @details So it means, it is used in the server code.
 *
 * @param fd connection socket
 * @param[out] message read message
 * @return 0 if success, -1 otherwise
 */
int read_c2s(int fd, bingo_message_c2s* message);

/**
 * @brief Write server-to-client message from the socket.
 * @details So it means, it is used in the server code.
 *
 * @param fd connection socket
 * @param message message to write
 * @return 0 if success, -1 otherwise
 */
int write_s2c(int fd, bingo_message_s2c* message);

/**
 * @brief Read server-to-client message from the socket.
 * @details So it means, it is used in the client code.
 *
 * @param fd connection socket
 * @param[out] message read message
 * @return 0 if success, -1 otherwise
 */
int read_s2c(int fd, bingo_message_s2c* message);

/**
 * @brief Write client-to-server message from the socket.
 * @details So it means, it is used in the client code.
 *
 * @param fd connection socket
 * @param message message to write
 * @return 0 if success, -1 otherwise
 */
int write_c2s(int fd, bingo_message_c2s* message);
