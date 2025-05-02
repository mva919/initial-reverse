#include <errno.h>
#include <fcntl.h>
#include <stdatomic.h>
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
  FILE *in_file = stdin, *out_file = stdout;

  if (argc > 1) {
    in_file = fopen(argv[1], "r");
    if (in_file == NULL) {
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

    out_file = fopen(argv[2], "w");
    if (out_file == NULL) {
      fprintf(stderr, "error: cannot open or create file '%s'\n", argv[2]);
      exit(1);
    }
  }

  unsigned int stack_i = 0, stack_size = STACK_SIZE_INC;
  char **stack = malloc(sizeof(char *) * stack_size);
  if (stack == NULL) {
    int err = errno;
    fprintf(stderr, "error %d: allocating initial stack\n", err);
    exit(1);
  }

  char *lineptr = NULL;
  size_t lineptr_size = 0;
  ssize_t chars_read;
  while ((chars_read = getline(&lineptr, &lineptr_size, in_file)) != -1) {
    // if we are reading from stdin and user gives empty line we stop taking in
    // input
    if (strcmp(lineptr, "\n") == 0) {
      break;
    }
    stack[stack_i] = malloc(chars_read + 1); // chars read doesn't include \0
    strncpy(stack[stack_i], lineptr, chars_read);
    lineptr = NULL;
    stack_i++;

    // resize stack if it get full
    if (stack_i == stack_size) {
      stack_size += STACK_SIZE_INC;
      stack = realloc(stack, sizeof(char *) * stack_size);
      if (stack == NULL) {
        int err = errno;
        fprintf(stderr, "error %d: reallocating stack\n", err);
        exit(1);
      }
    }
  }
  stack_i--; // stack pointer is one ahead of last item

  if (in_file != stdin) {
    fclose(in_file);
  }

  for (int i = (int)stack_i; i >= 0; i--) {
    if (out_file == stdout) {
      printf("%s", stack[i]);
    } else {
      fputs(stack[i], out_file);
    }
  }

  if (out_file != stdout) {
    fclose(out_file);
  }

  return 0;
}
