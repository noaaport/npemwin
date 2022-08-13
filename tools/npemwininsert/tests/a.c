/*
 * Test program: reads from the fifo and writes the input to stdout.
 *
 * Run:
 *
 * In one terminal execute
 * 
 * ./aout > output
 *
 * and in another terminal
 *
 * /npemwininsert -f infeed.fifo pfmsjuju tjsj_foca52-pfmsju.578535.txt
 *
 */ 
#include <unistd.h>
#include <stdio.h>
#include <err.h>
#include <fcntl.h>

#define DATA_SIZE 1116

int main(void) {

  
  char data[DATA_SIZE];
  int status = 0;
  int fd;
  int n;

  /* 
   * In the npemwin server, it should be opened O_RDWR so that
   * the descriptor remains opened even when there no nobody
   * is writing to it.  The read will bock in both cases, unless
   * we open it nonblock, but the open will not block with O_RDWR and
   * will block with O_RDONLY.
   */
  fd = open("infeed.fifo", O_RDWR);
  if(fd == -1)
    err(1, "%s\n", "open");

  fprintf(stdout, "%s\n", "waiting");

  while((n = read(fd, data, DATA_SIZE)) > 0) {
    write(STDOUT_FILENO, data, n);
  }

  if(n == -1)
    err(1, "%s\n", "read");

  close(fd);
  
  return(0);
}
