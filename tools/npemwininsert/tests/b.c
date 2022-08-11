/*
 * Test program to read the "output" from a.c, and writes to stdout
 * the decoded file, which consists of the original file with the
 * emwin headers inserted in ecah block and the initial and trailing NULLs.
 *
 * ./b.out > original
 */
#include <unistd.h>
#include <stdio.h>
#include <err.h>
#include <fcntl.h>
#include "../file.h"

#define DATA_SIZE 102400

int main(void) {
  
  char data[DATA_SIZE];
  int status = 0;
  int fd;
  int n;
  int i;
  char *p;
  off_t size;
  
  fd = open("output", O_RDONLY);
  if(fd == -1)
    err(1, "%s\n", "open");
  
  n = read(fd, data, DATA_SIZE);
  close(fd);
  
  if(n == -1)
      err(1, "%s\n", "read");

  p = &(data[0]);
  for(i = 0; i < n; ++i){
    p[i] = p[i] ^ 0xff;
  }

  write(STDOUT_FILENO, data, n);

  return(0);
}
