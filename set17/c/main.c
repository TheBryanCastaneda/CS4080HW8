#include <stdio.h>

#include "compiler.h"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: parser \"expression\"\n");
    return 64;
  }

  if (!compile(argv[1])) {
    return 65;
  }

  printf("Parse successful.\n");
  return 0;
}