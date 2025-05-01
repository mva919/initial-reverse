#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <unistd.h>

#define BUF_SIZE 1024
#define STACK_SIZE_INC 512

int main(int argc, char *argv[]) {
  if (argc > 3) {
    printf("usage: reverse <input> <output>\n");
    exit(1);
  }

  int inputfd = STDIN_FILENO, outputfd = STDOUT_FILENO;

  if (argc > 1) {
    inputfd = open(argv[1], O_RDONLY);
    if (inputfd == -1) {
      fprintf(stderr, "error: cannot open file '%s'\n", argv[1]);
      exit(1);
    }
  }

  if (argc > 2) {
    // check for same file used as input and output
    if (strcmp(argv[1], argv[2]) == 0) {
      fprintf(stderr, "error: cannot use same file '%s' for input and output\n",
              argv[1]);
      exit(1);
    }

    outputfd = open(argv[2], O_WRONLY);
    if (outputfd == -1) {
      fprintf(stderr, "error: cannot open file '%s'\n", argv[2]);
      exit(1);
    }
  }

  char buf[BUF_SIZE];
  char **stack = malloc(sizeof(char *) * STACK_SIZE_INC);
  if (stack == NULL) {
    int err = errno;
    fprintf(stderr, "error %d: allocataing initial stack\n", err);
    exit(1);
  }
  unsigned int stack_i = 0, cur_buf_size = 0, stack_size = STACK_SIZE_INC;
  int bytes_read = -1;

  for (;;) {
    bytes_read = read(inputfd, buf + cur_buf_size, BUF_SIZE);

    if (bytes_read == -1) {
      int err = errno;
      fprintf(stderr, "error %d: could not read from file\n", err);
      exit(1);
    }

    if (bytes_read == 1 && buf[cur_buf_size] == '\n') {
      stack_i -= 1; // decrementing stack pointer so we always point to last
                    // item in stack
      break;
    }

    char *ptr = malloc(bytes_read + 1); // strcpy needs size len(src) + 1 to
                                        // insert null terminating character
    if (ptr == NULL) {
      int err = errno;
      fprintf(stderr, "error %d: allocating read string\n", err);
      exit(1);
    }
    strcpy(ptr, buf + cur_buf_size);

    stack[stack_i++] = ptr;
    if (stack_i == stack_size) {
      // TODO: double stack size and copy over memory to new stack
    }
    cur_buf_size = (cur_buf_size + bytes_read) % BUF_SIZE;
  }

  for (int i = (int)stack_i; i >= 0; i--) {
    write(outputfd, stack[i], strlen(stack[i]));
  }

  return 0;
}
