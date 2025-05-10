/**
 * find.c - A file search utility for xv6 operating system
 *
 * This program implements a recursive file search functionality similar to the
 * Unix 'find' command. It searches for a specific filename within a given
 * directory path and its subdirectories.
 *
 * Algorithm Overview:
 *
 * 1. Start at the given path
 * 2. For each entry in the current directory:
 *    ├── If it's a file:
 *    │   └── Compare with target filename
 *    │       ├── Match → Print full path
 *    │       └── No match → Continue
 *    │
 *    └── If it's a directory:
 *        ├── Skip "." and ".."
 *        └── Recursively search subdirectory
 *
 * The algorithm uses depth-first search (DFS) to traverse the directory tree.
 */

#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"
#include "kernel/stat.h"
#include "user/user.h"

/**
 * Extracts the filename from a given path
 * @param path The full path string
 * @return Pointer to the filename portion of the path
 *
 * Example:
 * Input: "/usr/bin/file.txt"
 * Output: "file.txt"
 */
const char* get_path_name(const char* path) {
  const char* p;

  // Find first character after last slash.
  for (p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  return p;
}

/**
 * Recursively searches for a file in a directory and its subdirectories
 * @param path The directory path to search in
 * @param filename The name of the file to find
 */
void find(const char* path, const char* filename) {
  // Buffer for constructing full paths
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  // Open the directory
  if ((fd = open(path, O_RDONLY)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  // Get file status
  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch (st.type) {
    case T_DEVICE:
    case T_FILE:
      // If it's a file, check if it matches the target filename
      if (strcmp(get_path_name(path), filename) == 0) {
        fprintf(1, "%s\n", path);
      }
      break;
    case T_DIR:
      // If it's a directory, check path length and prepare for recursion
      if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
        fprintf(2, "find: path too long\n");
        break;
      }
      // Construct the base path for subdirectories
      strcpy(buf, path);
      p = buf + strlen(buf);
      *p++ = '/';

      // Read directory entries
      while (read(fd, &de, sizeof(de)) == sizeof(de)) {
        if (de.inum == 0)
          continue;
        // Construct full path for current entry
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if (stat(buf, &st) < 0) {
          fprintf(2, "find: cannot stat %s\n", buf);
          continue;
        }

        const char* name = get_path_name(buf);

        // Skip "." and ".." directories to avoid infinite recursion
        if (st.type == T_DIR && strcmp(name, ".") != 0 &&
            strcmp(name, "..") != 0) {
          find(buf, filename);  // Recursive call for subdirectories
        } else if (st.type == T_FILE && strcmp(name, filename) == 0) {
          fprintf(1, "%s\n", buf);  // Found a matching file
        }
      }
      break;
  }
  close(fd);
}

/**
 * Main function to handle command line arguments and start the search
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 * @return Exit status
 */
int main(int argc, char* argv[]) {
  if (argc != 3) {
    fprintf(2, "Usage: find <path> <filename>\n");
    exit(1);
  }

  const char* path = argv[1];
  const char* filename = argv[2];

  find(path, filename);

  exit(0);
}