/*
 * pingpong.c - A simple program demonstrating inter-process communication using
 * pipes
 *
 * This program creates a parent-child process pair that communicate through a
 * pipe. The parent sends a "ping" message to the child, and the child responds
 * with a "pong".
 *
 * Communication Flow:
 * 1. Parent creates a pipe
 * 2. Parent forks a child process
 * 3. Both processes have access to the pipe's read and write ends
 * 4. Parent writes "p" to pipe and waits for response
 * 5. Child reads "p" from pipe, prints "received ping", and writes "p" back
 * 6. Parent reads "p" from pipe and prints "received pong"
 *
 * Timing Diagram:
 *
 * Parent Process          Child Process
 *     |                       |
 *     |--pipe creation------>|
 *     |                       |
 *     |--fork()------------->|
 *     |                       |
 *     |--write("p")--------->|
 *     |                       |
 *     |<--read("p")----------|
 *     |                       |
 *     |<--write("p")---------|
 *     |                       |
 *     |--read("p")---------->|
 *     |                       |
 *     |--wait()------------->|
 *     |                       |
 *     |<--exit()-------------|
 *     |                       |
 */

#include "kernel/types.h" // Include kernel type definitions
#include "user/user.h"    // Include user-level system call definitions

int main(int argc, char *argv[]) {
  int p[2]; // Array to store pipe file descriptors
  pipe(p);  // Create a pipe, p[0] for reading, p[1] for writing

  char buf[1]; // Buffer to store single character messages

  if (fork() == 0) {            // Child process
    int n = read(p[0], buf, 1); // Read "p" from parent
    if (n > 0) {
      printf("%d: received ping\n", getpid()); // Print child's PID and message
      write(p[1], "p", 1);                     // Send "p" back to parent
    } else {
      printf("child: read error\n");
    }
    close(p[0]);                // Close read end
    close(p[1]);                // Close write end
    exit(n == 0 ? 1 : 0);       // Exit child process
  } else {                      // Parent process
    write(p[1], "p", 1);        // Send "p" to child
    int n = read(p[0], buf, 1); // Wait for child's response
    if (n > 0) {
      printf("%d: received pong\n", getpid()); // Print parent's PID and message
    } else {
      printf("parent: read error\n");
    }
    wait(0);              // Wait for child to exit
    close(p[0]);          // Close read end
    close(p[1]);          // Close write end
    exit(n == 0 ? 1 : 0); // Exit parent process
  }
  return 0;
}
