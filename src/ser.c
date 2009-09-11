/*
 * Copyright (c) 2005-2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifdef TEST
#include <stdio.h>
#include <err.h>
#endif
#include <assert.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "strsplit.h"
#include "ser.h"

#define SER_OPEN_FLAGS (O_RDONLY | O_NOCTTY | O_NONBLOCK)

struct ser_speed_st {
  char *s;
  speed_t speed;
};

struct ser_flags_st {
  char *s;
  tcflag_t flags;
};

struct ser_settings_st {
  speed_t speed;
  tcflag_t parity;
  tcflag_t bits;
  tcflag_t stopbits;
};

static speed_t str2speed(char *s);
static tcflag_t str2parity(char *s);
static tcflag_t str2bits(char *s);
static tcflag_t str2stopbits(char *s);
static int ser_get_settings(char *str, struct ser_settings_st *settings);

static struct ser_speed_st ser_speed[] = {
  {"50", B50},
  {"75", B75},
  {"110", B110},
  {"134", B134},
  {"150", B150},
  {"200", B200},
  {"300", B300},
  {"600", B600},
  {"1200", B1200},
  {"1800", B1800},
  {"2400", B2400},
  {"4800", B4800},
  {"9600", B9600},
  {"19200", B19200},
  {"38400", B38400},
  {NULL, 0}
};

static struct ser_flags_st ser_parity[] = {
  {"N", 0},
  {"n", 0},
  {"E", PARENB},
  {"e", PARENB},
  {"O", PARENB | PARODD},
  {"o", PARENB | PARODD},
  {NULL, 0}
};

static struct ser_flags_st ser_bits[] = {
  {"5", CS5},
  {"6", CS6},
  {"7", CS7},
  {"8", CS8},
  {NULL, 0}
};

static struct ser_flags_st ser_stopbits[] = {
  {"1", 0},
  {"2", CSTOPB},
  {NULL, 0}
};

static speed_t str2speed(char *s){

  speed_t r = 0;
  struct ser_speed_st *st = ser_speed;

  while(st->s != NULL){
    if(strcmp(s, st->s) == 0){
      r = st->speed;
      break;
    }
    ++st;
  }

  return(r);
}

static tcflag_t str2parity(char *s){

  tcflag_t r = 0;
  struct ser_flags_st *st = ser_parity;

  while(st->s != NULL){
    if(strcmp(s, st->s) == 0){
      r = st->flags;
      break;
    }
    ++st;
  }

  return(r);
}

static tcflag_t str2bits(char *s){

  tcflag_t r = 0;
  struct ser_flags_st *st = ser_bits;

  while(st->s != NULL){
    if(strcmp(s, st->s) == 0){
      r = st->flags;
      break;
    }
    ++st;
  }

  return(r);
}

static tcflag_t str2stopbits(char *s){

  tcflag_t r = 0;
  struct ser_flags_st *st = ser_stopbits;

  while(st->s != NULL){
    if(strcmp(s, st->s) == 0){
      r = st->flags;
      break;
    }
    ++st;
  }

  return(r);
}

static int ser_get_settings(char *str, struct ser_settings_st *settings){

  struct strsplit_st *strp;

  strp = strsplit_create(str, ",", STRSPLIT_FLAG_IGNEMPTY);
  if(strp == NULL)
    return(-1);

  if(strp->argc != 4){
    strsplit_delete(strp);
    return(-2);
  }

  settings->speed = str2speed(strp->argv[0]);
  settings->parity = str2parity(strp->argv[1]);
  settings->bits = str2bits(strp->argv[2]);
  settings->stopbits = str2stopbits(strp->argv[3]);

  strsplit_delete(strp);

  return(0);
}

int ser_open_port(char *device, char *settings_str){
  /*
   * Returns:
   *
   * -2 (from ser_get_settings) => invalid settings str.
   * -1 system error
   * fd
   */
  int fd;
  int status;
  struct termios tios;
  struct ser_settings_st settings;
  int ioctl_flags;

  assert((device != NULL) && (settings_str != NULL));

  status = ser_get_settings(settings_str, &settings);
  if(status != 0)
    return(status);

  if((fd = open(device, SER_OPEN_FLAGS, 0)) == -1){
     return(-1);
  }

  /*
   * Just make sure that all those that are not implementation
   * dependent are cleared (Levine, p.155).
   * Also http://www.easysw.com/~mike/serial/serial.html
   */
  tcgetattr(fd, &tios);  

  tios.c_iflag &= ~(IXON | ICRNL);
  tios.c_lflag &= ~( ECHO | ECHOE | ECHOK | ECHONL |
                     ICANON | ISIG | NOFLSH | TOSTOP | IEXTEN);
  tios.c_cflag &= ~(PARENB | PARODD | CSIZE | CSTOPB);

  tios.c_cflag |= (settings.parity | settings.bits | settings.stopbits |
    CLOCAL | CREAD);

  /*
   * We will use select() instead.
   *
   * tios.c_cc[VMIN] = 0;
   * tios.c_cc[VTIME] = 10;
  */

  cfsetospeed(&tios, settings.speed);
  cfsetispeed(&tios, settings.speed);

  if(tcsetattr(fd, TCSAFLUSH, &tios) == -1){
    close(fd);
    return(-1);
  }

  /*
   * This comment is carried over from the emwin-0.93 code:
   *
   * ``The DTR line is automatically raised but we need to lower the RTS
   * line to read anything from the EMWIN satellite modem.''
   *
   * I am following the advide here, implemented somewhat differently;
   * why is this needed anyway?
   */
  /*
  sleep(1);
  ioctl_flags = TIOCM_RTS; 
  if(ioctl(fd, TIOCMBIC, &ioctl_flags) != 0){
    close(fd);
    return(-1);
  }
  */

  status = ioctl(fd, TIOCMGET, &ioctl_flags);
  if(status == 0){
    ioctl_flags &= ~TIOCM_RTS; 
    status = ioctl(fd, TIOCMSET, &ioctl_flags);
  }
  if(status != 0){
    close(fd);
    return(-1);
  }

  tcflush(fd, TCIFLUSH);

  return(fd);
}

int ser_close_port(int fd){

  return(close(fd));
}

#ifdef TEST
int main(int argc, char ** argv){

  struct ser_settings_st settings;
  int status = 0;

  status = ser_get_settings(argv[1], &settings);
  if(status == 1)
    errx(1, "status = 1");
  else if(status == -1)
    err(1, "get_ser_settings()");

  fprintf(stdout, "%u\n", (unsigned int)settings.speed);
  fprintf(stdout, "%u\n", (unsigned int)settings.parity);
  fprintf(stdout, "%u\n", (unsigned int)settings.bits);
  fprintf(stdout, "%u\n", (unsigned int)settings.stopbits);

  return(0);
}
#endif
