#include <stdio.h>
#include "error.h"

void error_handling(char* buf) {
  fputs(buf, stderr);
  fputc('\n', stderr);
  exit(1);
}
