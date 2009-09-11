/*
 * This was used to to check the V2 protocol.
 *
 * Usage: ./a.out < OUT
 *
 * where OUT is a full V2 queue packet as if a call to
 *
 * write(fd, ep->queue_data_v2, ep->queue_data_v2_size);
 *
 * was inserted at the end of build_queue_data_v2() in v2.c
 *
 */
#include <stdio.h>
#include <err.h>
#include <unistd.h>
#include <fcntl.h>
#include <zlib.h>

static void write_one(char *fname, char *data, size_t data_size);

int main(void){

  int c;
  unsigned char b;
  char qpacket[1116 + 4];
  char *rawdata;
  char *block;
  int qpacket_size;
  int rawdata_size;
  int block_size;
  char original[1024];
  int original_size = 1024;
  int i;

  i = 0;
  while((c = fgetc(stdin)) != EOF){
    b = (unsigned char)c;
    qpacket[i++] ^= b ^ 0xff;
  }
  qpacket_size = i;
  rawdata = &qpacket[4 + 6];
  block = &rawdata[80];
  rawdata_size = qpacket_size - 10;
  block_size = rawdata_size - 80;

  fprintf(stdout, "%d %d %d\n", qpacket_size, rawdata_size, block_size);
  
  write_one("PACKET", &qpacket[4], (size_t)(qpacket_size - 4));
  write_one("RAW", rawdata, (size_t)rawdata_size);
  write_one("BLOCK", block, (size_t)block_size);

  uncompress((Bytef*)original, (uLongf*)&original_size,
	     (Bytef*)block, (uLongf)block_size);

  write_one("ORIG", original, original_size);

  return(0);
}

static void write_one(char *fname, char *data, size_t data_size){

  int fd;

  fd = open(fname, O_WRONLY | O_CREAT, 0644);
  write(fd, data, data_size);
  close(fd);
}
