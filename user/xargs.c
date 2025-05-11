/**
 * xargs.c - A simplified implementation of the Unix xargs command
 *
 * This module implements a basic version of the xargs command, which reads
 * items from standard input, delimited by newlines, and executes a command for
 * each item.
 *
 * Algorithm Description:
 * 1. Read command-line arguments (the command to execute)
 * 2. Read input lines from stdin
 * 3. For each input line:
 *    - Construct argument list by combining command args and input line
 *    - Fork a child process
 *    - Execute the command in child process
 *    - Parent waits for child to complete
 *
 * Sequence Diagram:
 *
 * Parent Process                    Child Process
 *     |                                |
 *     |-- fork() --------------------> |
 *     |                                |
 *     |<-- exec(command, args) ------- |
 *     |                                |
 *     |-- wait() --------------------> |
 *     |                                |
 *     |<-- exit() -------------------- |
 *     |                                |
 *
 * @author: xv6-labs
 * @version: 1.0
 */

#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"
#include "kernel/fcntl.h"

/**
 * Reads a single line from standard input into the provided buffer
 *
 * @param buf: Buffer to store the read line
 * @param max: Maximum number of characters to read
 * @return: Number of characters read (excluding null terminator)
 *
 * This function reads characters one by one from stdin until either:
 * - A newline character is encountered
 * - The maximum buffer size is reached
 * - End of input is reached
 */
int readline_from_stdin(char* buf, int max) {
  int n = 0;
  while (n < max && read(0, buf + n, 1) > 0) {
    if (buf[n] == '\n') {
      break;
    }
    n++;
  }

  buf[n] = '\0';
  return n;
}

/**
 * Main function implementing the xargs command
 *
 * Usage: xargs command [arg1 arg2 ...]
 *
 * The program:
 * 1. Validates command-line arguments
 * 2. Reads input lines from stdin
 * 3. For each line:
 *    - Constructs argument list
 *    - Executes command with arguments
 *
 * @param argc: Number of command-line arguments
 * @param argv: Array of command-line argument strings
 * @return: 0 on success, 1 on error
 */
int main(int argc, char* argv[]) {
  // Validate minimum number of arguments
  if (argc < 2) {
    fprintf(2, "usage: xargs command\n");
    exit(1);
  }

  // Check if number of arguments exceeds system limit
  // MAXARG - 1 because we need space for the command and input line
  if (argc > MAXARG - 1) {
    fprintf(2, "xargs: too many arguments\n");
    exit(1);
  }

  char buf[1024];      // Buffer for reading input lines
  char* args[MAXARG];  // Array to hold command arguments
  args[0] = argv[1];   // First argument is the command to execute

  // Process each line from stdin
  while (readline_from_stdin(buf, sizeof(buf) / sizeof(buf[0])) > 0) {
    // Copy command-line arguments to args array
    for (int i = 2; i < argc; i++) {
      args[i - 1] = argv[i];
    }
    args[argc - 1] = buf;  // Add the input line as the last argument
    args[argc] = 0;        // Null terminate the argument list

    // Create child process to execute command
    if (fork() == 0) {
      exec(argv[1], args);  // Execute command with constructed arguments
      fprintf(2, "xargs: exec %s failed\n", argv[1]);
    }

    wait(0);  // Parent waits for child to complete
  }

  exit(0);
}
