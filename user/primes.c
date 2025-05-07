/*
 * Prime Number Sieve using Pipes
 * 
 * This program implements the Sieve of Eratosthenes algorithm using pipes and processes
 * to find prime numbers concurrently. The algorithm works as follows:
 * 
 * 1. Each process represents a prime number and filters out its multiples
 * 2. Numbers are passed through pipes between processes
 * 3. Each process reads numbers from its left pipe and writes non-multiples to its right pipe
 * 
 * Process Communication Flow:
 * 
 * Process 1 (2) -> Process 2 (3) -> Process 3 (5) -> Process 4 (7) -> ...
 *    [pipe1]         [pipe2]         [pipe3]          [pipe4]
 * 
 * Each process:
 * 1. Reads numbers from left pipe
 * 2. If number is not divisible by its prime, passes it to right pipe
 * 3. Creates new child process for first number it receives
 * 
 * Example flow for numbers 2-10:
 * 
 * Time   Process 1    Process 2    Process 3    Process 4
 * ---------------------------------------------------------
 * t0      reads 2      -            -            -
 * t1      sends 3      reads 3      -            -
 * t2      sends 5      sends 5      reads 5      -
 * t3      sends 7      sends 7      sends 7      reads 7
 * t4      sends 9      reads 9      -            -
 * 
 * Note: Numbers 4, 6, 8, 10 are filtered out by Process 1 (multiples of 2)
 *       Numbers 9 is filtered out by Process 2 (multiple of 3)
 *       Only prime numbers reach their corresponding processes
 */

#include "kernel/types.h"
#include "user/user.h"

// Constants for prime number calculation
#define START_PRIME 2        // First prime number to start with
#define MAX_PRIMES 35       // Maximum number to check for primes

// Pipe file descriptor constants
#define PIPE_INVALID -1      // Invalid pipe descriptor
#define PIPE_READ 0          // Read end of pipe
#define PIPE_WRITE 1         // Write end of pipe

/**
 * Prints a prime number to the console
 * @param n The prime number to print
 */
void print_prime(int n) { printf("prime %d\n", n); }

/**
 * Safely closes a pipe file descriptor if it's valid
 * @param p The pipe file descriptor to close
 */
void close_pipe_if_valid(int p) {
  if (p != PIPE_INVALID) {
    close(p);
  }
}

/**
 * Delivers a number through the pipe system and creates new processes for primes
 * @param n The number to process
 * @param pipe_left The left pipe for receiving numbers
 */
void deliver_prime(int n, int pipe_left[2]) {
  // If pipe is not initialized, create it and fork a new process
  if (pipe_left[PIPE_WRITE] == PIPE_INVALID) {
    int ret = pipe(pipe_left);
    if (ret < 0) {
      printf("pipe error, current index: %d\n", n);
      exit(1);
    }

    ret = fork();
    if (ret == 0) {
      // Child process
      close_pipe_if_valid(pipe_left[PIPE_WRITE]);

      // Print the prime number this process represents
      print_prime(n);

      // Initialize right pipe for passing numbers to next process
      int pipe_right[2] = {PIPE_INVALID, PIPE_INVALID};
      int received_number = 0;

      // Read numbers from left pipe and filter them
      while (read(pipe_left[PIPE_READ], &received_number,
                  sizeof(received_number)) > 0) {
        // Skip numbers that are multiples of current prime
        if (received_number % n == 0) {
          continue;
        }

        // Pass non-multiples to next process
        deliver_prime(received_number, pipe_right);
      }

      // Clean up pipes
      close_pipe_if_valid(pipe_left[PIPE_READ]);
      close_pipe_if_valid(pipe_right[PIPE_READ]);
      close_pipe_if_valid(pipe_right[PIPE_WRITE]);

      // Wait for child process to complete
      wait(0);
      exit(0);
    } else if (ret > 0) {
      // Parent process continues
    } else {
      printf("fork error, current index: %d\n", n);
      exit(1);
    }
  } else {
    // printf("deliver_prime: %d\n", n);
    // Write number to pipe
    if (write(pipe_left[PIPE_WRITE], &n, sizeof(n)) <= 0) {
      exit(1);
    }
  }
}

/**
 * Main function that initiates the prime number calculation
 * @param argc Number of command line arguments
 * @param argv Command line arguments
 * @return 0 on successful execution
 */
int main(int argc, char *argv[]) {
  // Initialize pipe for first process
  int p[2] = {PIPE_INVALID, PIPE_INVALID};

  // Print the first prime number
  print_prime(START_PRIME);

  // Process numbers from START_PRIME + 1 to MAX_PRIMES
  for (int i = START_PRIME + 1; i <= MAX_PRIMES; i++) {
    // Skip multiples of START_PRIME
    if (i % START_PRIME == 0) {
      continue;
    }

    // Process the number through the pipe system
    deliver_prime(i, p);
  }

  // Clean up pipes
  close(p[PIPE_READ]);
  close(p[PIPE_WRITE]);

  // Wait for child process to complete
  wait(0);

  return 0;
}
