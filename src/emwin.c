/*
 * Copyright (c) 2004-2012 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include "config.h"
#include <assert.h>
#include <stdio.h>
/*
 * This function is in <stdio.h>, but in solaris it requires some
 * extra defines to pull in the declaration. To avoid ading those defines
 * I will temporarily declare it explicitly.
 */
#ifdef SOLARIS
int putc_unlocked(int c, FILE *stream);
#endif
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>
#include <inttypes.h>
#include "libconnth/tcpsock.h"
#include "util.h"
#include "file.h"
#include "readn.h"
#include "globals.h"
#include "ser.h"
#include "bb.h"
#include "wx14.h"
#include "emwin.h"
#include "defaults.h"

static int emwin_sync_device(int fd);
static int emwin_sync_serial(int fd);
static int emwin_sync_wx14_msg(int fd);
static int emwin_sync_wx14_raw(int fd);
static int fill_packet_struct_bb(struct emwin_packet *ep,
				 char *bbdata, int datasize);
static int fill_packet_struct_wx14(struct emwin_packet *ep,
				   char *data, int datasize);
static int fill_packet_struct_serial(struct emwin_packet *ep, 
				     char *serialdata, int datasize);
static int checksum(struct emwin_packet *ep);
static int checkfilename(struct emwin_packet *ep);
static int is_text_file(char *filename);

static int try_serverlist(struct emwin_packet *ep,
			  char *bbdata, int bbdata_size);
static int write_server_list(char *s, char *fname);

static int read_qinfo(char *qfilename, struct emwin_file_qinfo *qinfo);
static int write_qinfo(char *qfilename, struct emwin_file_qinfo *qinfo,
		       mode_t mode);

int open_emwin_server_network(int type, char *ipstr, char *port,
			      int *gai_code){
  /*
   * The "type" argument is the wx14 or a bb server.
   *
   * There can be cases in which the connection is opened,
   * but the server closes before anything has been read (e.g., 
   * it happens if the server uses libwrap).
   * Therefore, after a successfull open(), we check that we can
   * receive a packet (which is discarded).
   *
   * Returns:
   * fd
   * -1 (system error)
   * -2 Some other connection error, timed out trying to get packet,
   *    or some other error from get_emwin_packet_xxx().
   */
  int status = 0;
  int fd = -1;
  struct emwin_packet ep;
  
  /*
   * We put a timer here to avoid getting stuck too long trying to
   * connect to a server. The OS default is something like 70 secs.
   * If g.connecttimeout_s is 0 or negative the configuration timeout
   * facility is disabled. Use the OS defaults for the so_snd/so_rcvbuf.
   */
  if(g.connecttimeout_s > 0)
    fd = tcp_client_open_conn_timed(ipstr, port, g.connecttimeout_s,
				    -1, -1, gai_code);
  else
    fd = tcp_client_open_conn(ipstr, port, -1, -1, gai_code);

  if(fd == -1)
    return(-1);

  if(type == EMWIN_SERVER_TYPE_WX14_MSG){
    status = emwin_sync_wx14_msg(fd);
  } else if(type == EMWIN_SERVER_TYPE_WX14_RAW){
    status = emwin_sync_wx14_raw(fd);
  } else
     status = get_emwin_packet_bb(fd, &ep);

  if(status != 0){
    close(fd);
    fd = -1;
    if(status != -1)
      fd = -2;
  }
  
  return(fd);
}

int open_emwin_server_serial(char *device, char *settings_str){
  /*
   * Returns:
   * fd
   * -1 (system error)
   * -2 Some other connection error, timed out trying to get packet,
   *    or some other error from ser_open_port(), or emwin_sync_serial().
   */
  int fd;
  int status = 0;

  fd = ser_open_port(device, settings_str);
  if(fd < 0)
    return(fd);

  status = emwin_sync_serial(fd);
  if(status != 0){
    close(fd);
    fd = -2;	/* Same as ser_open() non-system error */
  }

  return(fd);
}

int get_emwin_packet_bb(int f, struct emwin_packet *ep){

  int status = 0;
  size_t size;
  char bbdata[EMWIN_PACKET_SIZE];
  int  bbdata_size;

  assert(g.readtimeout_s >= 0);
  /*
   * Determine the type of packet from the first byte, then read
   * the rest accordingly.
   */
  status = bb_readn(f, bbdata, sizeof(char),
		      g.readtimeout_s, g.readtimeout_retry);
  if(status != 0)
    return(status);

  ep->bbtype = BB_PACKET_TYPE_UNKNOWN;

  /* What is left to read */
  size = EMWIN_PACKET_SIZE - sizeof(char);

  if(bb_is_emwin_data_packet(bbdata[0])){
    ep->bbtype = BB_PACKET_TYPE_DATA;
    bbdata_size = EMWIN_PACKET_SIZE;    
    status = bb_readn(f, &bbdata[1], size, g.readtimeout_s,
		      g.readtimeout_retry);
  } else if(bb_is_srvlist_packet(bbdata[0])){
    ep->bbtype = BB_PACKET_TYPE_SRVLIST;
    status = bb_read_srvlist(f, &bbdata[1], &size, g.readtimeout_s,
			     g.readtimeout_retry);
    bbdata_size = size + sizeof(char);
  } else {
    /* Let the caller know the character read */
    ep->bbdata[0] = bbdata[0];
    return(2);
  }

  if(status != 0)
    return(status);

  /*
   * These functions return error codes >= 2.
   */
  if(ep->bbtype == BB_PACKET_TYPE_DATA){
     status = fill_packet_struct_bb(ep, bbdata, bbdata_size);
  } else if(ep->bbtype == BB_PACKET_TYPE_SRVLIST){
    status = try_serverlist(ep, bbdata, bbdata_size);
  }

  return(status);
}

int get_emwin_packet_wx14_msg(int f, struct emwin_packet *ep){

  int status = 0;
  size_t size = EMWIN_PACKET_SIZE;
  char data[EMWIN_PACKET_SIZE];

  ep->bbtype = BB_PACKET_TYPE_UNKNOWN;

  assert(g.readtimeout_s >= 0);

  status = wx14_read_emwin_block(f, g.readtimeout_s, 0, &g.wx14msg);

  if(status == 0){
    ep->bbtype = BB_PACKET_TYPE_DATA;
    status = wx14_memcpy_emwin_block(data, &size, &g.wx14msg);
  }

  if(status == 0)
    status = fill_packet_struct_wx14(ep, data, (int)size);

  return(status);
}

int get_emwin_packet_wx14_raw(int f, struct emwin_packet *ep){

  return(get_emwin_packet_serial(f, ep));
}

int get_emwin_packet_serial(int f, struct emwin_packet *ep){
  /*
   * Call fill_packet_struct_serial(ep, bbdata, datasize);
   */
  int status = 0;
  ssize_t datasize;
  char serialdata[EMWIN_PACKET_SIZE];

  ep->bbtype = BB_PACKET_TYPE_UNKNOWN;

  assert(g.readtimeout_s >= 0);
  datasize = readn(f, serialdata, EMWIN_PACKET_SIZE, g.readtimeout_s, 0);

  if(datasize != EMWIN_PACKET_SIZE){
    if(datasize == -1)
      status = -1;
    else if(datasize == -2)
      status = -2;	/* timed out (poll) before reading anything */
    else if(datasize == 0)
      status = -3;	/* eof (server disconnected) */
    else
      status = 1;	/* short read (timedout or disconnect) while reading */
  }

  if(status == 0){
    ep->bbtype = BB_PACKET_TYPE_DATA;
    status = fill_packet_struct_serial(ep, serialdata, (int)datasize);
  }

  return(status);
}

static int emwin_sync_device(int fd){
  /*
   * In the case of the serial port the synhronization is harder.
   * We look for the set of four characters '\0'/PF. If we find it,
   * we assume it is the start of the header (past the first five of
   * the six initial '\0') and read the remaining (1116 - 5 - 4)
   * characters to read up to the end of the packet.
   * That packet is just discarded, and let the program continue from there.
   */
  int stage = -1;
  char c;
  char serialdata[EMWIN_PACKET_SIZE];
  ssize_t datasize;
  int nloop = 0;

  /*
   * 4464 => the equivalent of four packets.
   */
  while((stage != 3) && (nloop < (EMWIN_PACKET_SIZE * 4))){
    ++nloop;

    if(readn(fd, &c, sizeof(char), g.readtimeout_s, 0) != 
       (ssize_t)sizeof(char)){

      return(-1);
    }

    switch(c){
    case '\0':
      stage = 0;
      break;
    case '/':
      if(stage == 0)
	++stage;
      else
	stage = -1;
      
      break;
    case 'P':
      if(stage == 1)
	++stage;
      else
	stage = -1;
      
      break;
    case 'F':
      if(stage == 2)
	++stage;
      else
	stage = -1;
      
      break;
    default:
      stage = -1;
      break;
    }
  }

  if(stage != 3)
    return(-1);

  datasize = readn(fd, serialdata, EMWIN_PACKET_SIZE - 9, g.readtimeout_s, 0);
  if(datasize != EMWIN_PACKET_SIZE - 9)
    return(-1);

  return(0);
}

static int emwin_sync_serial(int fd){
  /*
   * After synczing, try to get a packet and "decode" it. If
   * get_emwin_packet_serial() returns ok, or a read error (status <= 1),
   * then exit. Otherwise (which means fill_packet returnes a format error),
   * resync and repeat.
   */
  struct emwin_packet ep;
  int status = 0;
  int i = 0;

  status = emwin_sync_device(fd);

  /*
   * Reuse "g.readtimeout_retry" here
   */
  while((i <= g.readtimeout_retry) && (status == 0)){
    status = get_emwin_packet_serial(fd, &ep);
    if(status < 2)
      break;
    else
      status = emwin_sync_device(fd);
    
    ++i;
  }

  return(status);
}

static int emwin_sync_wx14_msg(int fd){
  /*
   * The strategy here is the same as in the function
   *
   * emwin_sync_serial()
   *
   * above
   */
  struct emwin_packet ep;
  int status = 0;
  int i = 0;

  wx14_init(&g.wx14msg);
  status = wx14_init_emwin_block(fd, g.readtimeout_s, g.readtimeout_retry,
				 &g.wx14msg);

  /*
   * Reuse "g.readtimeout_retry" here
   */
  while((i <= g.readtimeout_retry) && (status == 0)){
    status = get_emwin_packet_wx14_msg(fd, &ep);
    if(status != WX14_ERROR_EMWIN_FILL_PACKET)
      break;
    else {
      wx14_init(&g.wx14msg);
      status = wx14_init_emwin_block(fd, g.readtimeout_s, g.readtimeout_retry,
				     &g.wx14msg);
    }

    ++i;
  }

  return(status);
}

static int emwin_sync_wx14_raw(int fd){
  /*
   * The strategy here is the same as in the function
   *
   * emwin_sync_serial()
   *
   * above
   */
  struct emwin_packet ep;
  int status = 0;
  int i = 0;

  wx14_init(&g.wx14msg);
  status = emwin_sync_device(fd);

  /*
   * Reuse "g.readtimeout_retry" here
   */
  while((i <= g.readtimeout_retry) && (status == 0)){
    status = get_emwin_packet_wx14_raw(fd, &ep);
    if(status < 2)
      break;
    else
      status = emwin_sync_device(fd);
    
    ++i;
  }

  return(status);
}

static int fill_packet_struct_bb(struct emwin_packet *ep, 
				      char *bbdata, int datasize){
  /*
   * The width of the string fields in fmt depend on what is defined
   * in emwin.h, for the length of the file name and the time stamp.
   * If the file name has exactly 12 characters this format PF%12s/PN is
   * correct. But if it has less, and the rest are blanks, the /PN
   * will not match.  For that reason we write it as "PF%12s /PN"
   * because that space before the /PN is actually discarded.
   */  
  int i;
  /*   char *fmt = "/PF%12s/PN %d /PT %d /CS %d /FD%31s"; */
  char *fmt = "/PF%12s /PN %d /PT %d /CS %d /FD%22c";
  char *b;

  /*
   * This (ep->bbdata) is the bb stuff that we get and what we will retransmit.
   * We should really retransmit the cleaned stuff (but of course
   * that would not a BB server). On the other hand, if we retransmit
   * just the cleaned data (and write the program to be prepared
   * to get the data as such) we can reduce bandwidth significantly
   * by eliminating the transmission of unnecessary stuff
   * (NULL's, padding, etc) that these packets contain.
   */
  memcpy(ep->bbdata, bbdata, datasize);
  ep->bbdata_size = datasize;

  /* 
   * Get the unxored header + data block.
   * Get rid of the initial and final 6 NULL's
   * that are always present, and decode each character.
   * ep->rawdata then contains the unxored header + data block.
   */
  datasize -= (EMWIN_NULLPAD_SIZE * 2);
  for(i = 0; i < datasize; ++i){
    ep->rawdata[i] = bbdata[i + EMWIN_NULLPAD_SIZE] ^ 0xff;
  }
  ep->rawdata_size = datasize;
  
  /*
   * ep->data points to the start of the data block.
   */
  ep->data = &(ep->rawdata[EMWIN_HEADER_SIZE]);
  ep->data_size = datasize - EMWIN_HEADER_SIZE;

  /*
   * The V1 queue packet is just the bbdata, prepended by the size. The V2
   * depends on the raw (unxored) data.
   */
  memcpy(&ep->queue_data_v1[EMWIN_QUEUE_ENVELOPE_SIZE],
	 ep->bbdata, ep->bbdata_size);
  pack_uint32(ep->queue_data_v1, (uint32_t)ep->bbdata_size, 0);
  ep->queue_data_v1_size = EMWIN_QUEUE_ENVELOPE_SIZE + ep->bbdata_size;

  /*
   * For V2 we use the function in v2.c. If it fails, fall back to a V1
   * style queue packet as above.
   *
   * Moreover, if the configuration has specified a V1 only server, V2 clients
   * get V1 packages only.
   */
  if((g.serverprotocol == PROTOCOL_EMWIN1) || (build_queue_data_v2(ep) != 0) ||
     (ep->compress_ratio >= g.min_compress_ratio)){
    memcpy(&ep->queue_data_v2[EMWIN_QUEUE_ENVELOPE_SIZE],
	   ep->bbdata, ep->bbdata_size);
    pack_uint32(ep->queue_data_v2, (uint32_t)ep->bbdata_size, 0);
    ep->queue_data_v2_size = EMWIN_QUEUE_ENVELOPE_SIZE + ep->bbdata_size;
  }

  update_stats_frames_received((unsigned int)ep->bbdata_size);

  /*
   * Get the elements of the header.
   */  
  i = sscanf(ep->rawdata, fmt, ep->header.filename, &ep->header.blockno, 
	     &ep->header.numblocks, &ep->header.checksum, &ep->header.dtstamp);

  if(i < 5){
    update_stats_frames(1);
    return(2);	/* error in header format */
  }

  ep->header.dtstamp[EMWIN_TIMESTAMP_LEN - 1] = '\0';

  if(checksum(ep) != 0){
    update_stats_frames(1);
    return(3);
  }

  /*
   * This next function checks the file name and also converts it
   * to lower case.
   */
  if(checkfilename(ep) != 0){
    update_stats_frames(1);
    return(4);
  }

  /*
   *  log_verbose("%s: %d of %d", ep->header.filename,
   *	ep->header.blockno, ep->header.numblocks); 
   */

  /*
   * Sometimes, if the 1024 block does not contain enough data,
   * it is filled with NULL's. We get rid of them, but only if
   * it is txt file.
   */
  if(is_text_file(ep->header.filename)){

    b = &(ep->data[0]);
    i = 0;
    while((*b != '\0') && (i < ep->data_size)){
      ++i;
      ++b;
    }

    ep->data_size = i;
  }

  update_stats_frames(0);

  return(0);
}

static int fill_packet_struct_serial(struct emwin_packet *ep, 
				     char *serialdata, int datasize){
  int i;
  char *fmt = "/PF%12s /PN %d /PT %d /CS %d /FD%22c";
  char *b;

  /*
   * This (ep->bbdata) is the bb stuff that we get and what we will retransmit.
   * We should really retransmit the cleaned stuff (but of course
   * that would not a BB server).
   *
   * Note: Encrypt the data portion as well as the first and last six bytes.
   *       If those should not be encrypted, then the loop below should read
   *
   *	 memset(ep->bbdata, '\0', EMWIN_PACKET_SIZE);
   *     for(i = EMWIN_NULLPAD_SIZE;
   *		i < datasize - EMWIN_NULLPAD_SIZE; ++i){
   *		ep->bbdata[i] = serialdata[i] ^ 0xff;
   *	 }
   */
  /*  bb_xor(ep->bbdata, serialdata, data_size); */
  for(i = 0; i < datasize; ++i){
    ep->bbdata[i] = serialdata[i] ^ 0xff;
  }
  ep->bbdata_size = datasize;

  /* 
   * Get rid of the initial and final 6 NULL's that are always present.
   */
  datasize -= EMWIN_NULLPAD_SIZE * 2;
  for(i = 0; i < datasize; ++i){
    ep->rawdata[i] = serialdata[i + EMWIN_NULLPAD_SIZE];
  }
  ep->rawdata_size = datasize;

  /*
   * let ep->data point to the real data.
   */
  ep->data = &(ep->rawdata[EMWIN_HEADER_SIZE]);
  ep->data_size = datasize - EMWIN_HEADER_SIZE;

  /*
   * Form the V1 and V2 queue packets, as for the network case.
   */
  memcpy(&ep->queue_data_v1[EMWIN_QUEUE_ENVELOPE_SIZE],
	 ep->bbdata, ep->bbdata_size);
  pack_uint32(ep->queue_data_v1, (uint32_t)ep->bbdata_size, 0);
  ep->queue_data_v1_size = EMWIN_QUEUE_ENVELOPE_SIZE + ep->bbdata_size;

  if((g.serverprotocol == PROTOCOL_EMWIN1) || (build_queue_data_v2(ep) != 0) ||
     (ep->compress_ratio >= g.min_compress_ratio)){
    memcpy(&ep->queue_data_v2[EMWIN_QUEUE_ENVELOPE_SIZE],
	   ep->bbdata, ep->bbdata_size);
    pack_uint32(ep->queue_data_v2, (uint32_t)ep->bbdata_size, 0);
    ep->queue_data_v2_size = EMWIN_QUEUE_ENVELOPE_SIZE + ep->bbdata_size;
  }

  update_stats_frames_received((unsigned int)ep->bbdata_size);
  
  i = sscanf(ep->rawdata, fmt, ep->header.filename, &ep->header.blockno, 
	     &ep->header.numblocks, &ep->header.checksum, &ep->header.dtstamp);
  /*
   *  log_info("%d of %d", ep->header.blockno, ep->header.numblocks); 
   */

  if(i < 5){
    update_stats_frames(1);
    return(2);	/* error in header format */
  }

  ep->header.dtstamp[EMWIN_TIMESTAMP_LEN - 1] = '\0';

  if(checksum(ep) != 0){
    update_stats_frames(1);
    return(3);
  }

  /*
   * This next function checks the file name and also converts it
   * to lower case.
   */
  if(checkfilename(ep) != 0){
    update_stats_frames(1);
    return(4);
  }

  /*
   * Sometimes, if the 1024 block does not contain enough data,
   * it is filled with NULL's. We get rid of them, but only if
   * it is txt file.
   */
  if(is_text_file(ep->header.filename)){

    b = &(ep->data[0]);
    i = 0;
    while((*b != '\0') && (i < ep->data_size)){
      ++i;
      ++b;
    }

    ep->data_size = i;
  }

  update_stats_frames(0);

  return(0);
}

static int fill_packet_struct_wx14(struct emwin_packet *ep, 
				      char *data, int size){
  int status = 0;

  /* Handle it like the serial port data packet */
  status = fill_packet_struct_serial(ep, data, size);
  if(status != 0)
    status = WX14_ERROR_EMWIN_FILL_PACKET;

  return(status);
}

int save_emwin_packet(struct emwin_packet *ep){
  /*
   * If this is the first packet of a file (i.e., ep->header.blockno  == 1)
   * then a sequence number key is generated, the file's spool subdirectory
   * is created, the packet is saved and a temporary queue file is created.
   * If this is not the first packet, first the queue file is read. If
   * it does not exist it means that the file is not being processed
   * and the packet is discarded. If the queue file exists, then it is read
   * to get the relevant information, the packet is saved and the queue file
   * is updated. When the last packet is received, the temporary file
   * is renamed to the final queue file for the qrunner to process.
   */
  int status = 0;
  int f = -1;
  int n;
  mode_t mode = g.spoolfile_mode;
  time_t stamp;
  struct emwin_file_qinfo efqinfo;
  FILE *fq = NULL;

  n = snprintf(g.qfiles->q_filename, g.qfiles->qfn_size, "%s/%s/%s.t",
		 g.spooldir, g.qfilesdir, ep->header.filename);
  assert(n == g.qfiles->qfn_size - 1);

  /*
   * If this is not the first block, we try to open the 
   * (temporary) queue file and read the efqinfo. Otherwise
   * we initialize the efqinfo from the ep structure.
   */
  if(ep->header.blockno != 1){
    status = read_qinfo(g.qfiles->q_filename, &efqinfo);
    if(status == 1){
      /*
       * No queue file. The file is not being saved. Return without error.
       */
      status = 0;
      goto End;
    }else if(status == -1){
      goto End;
    }
  }else{
    /*
     * This is the first block.
     */
    strncpy(efqinfo.filename, ep->header.filename, EMWIN_TOTAL_FNAME_LEN);
    efqinfo.numblocks = ep->header.numblocks;
    efqinfo.rcvblocks = 0;
    if(get_seqnum_key(efqinfo.seqnumkey) != 0){
      status = -1;
      goto End;
    }
  }

  n = snprintf(g.qfiles->data_filename, g.qfiles->datafn_size, "%s/%s/%s.%d", 
	      g.spooldir, g.datadir, efqinfo.seqnumkey, ep->header.blockno); 
  assert(n <= g.qfiles->datafn_size - 1);

  n = snprintf(g.qfiles->tmp_filename, g.qfiles->tmpfn_size, "%s/%s/%s.t",
	       g.spooldir, g.tmpdir, ep->header.filename);
  assert(n == g.qfiles->tmpfn_size - 1);

  /*
   * write the data to the tmp file
   */
  f = open(g.qfiles->tmp_filename, O_WRONLY | O_CREAT, mode);
  if(f == -1){
    status = -1;
    goto End;
  }

  n = write(f, ep->data, ep->data_size);
  if(n < ep->data_size){
    status = -1;
    goto End;
  }

  close(f);
  f = -1;
  status = rename(g.qfiles->tmp_filename, g.qfiles->data_filename);
  if(status != 0)
    goto End;

  ++efqinfo.rcvblocks;

  /*
   * Update the temporary queue file.
   */
  status = write_qinfo(g.qfiles->q_filename, &efqinfo, mode);
  if(status != 0)
    goto End;

  /*
   * If this is last block, then write the qrunner q file,
   * reusing the tmp file, and delete the temporary qfile.
   */
  if(ep->header.blockno == ep->header.numblocks){
    unlink(g.qfiles->q_filename);
    n = snprintf(g.qfiles->q_filename, g.qfiles->qfn_size, "%s/%s/%s.q",
		 g.spooldir, g.qfilesdir, ep->header.filename);
    assert(n == g.qfiles->qfn_size - 1);

    stamp = time(NULL);
    if((fq = fopen(g.qfiles->tmp_filename, "w")) == NULL){
      status = -1;
      goto End;
    }

    n = fprintf(fq, "%s %s %d %d %" PRIuMAX, 
		efqinfo.filename, efqinfo.seqnumkey, 
		efqinfo.numblocks, efqinfo.rcvblocks, (uintmax_t)stamp);

    fclose(fq);
    fq = NULL;
    if(n < 0){
      status = -1;
      goto End;
    }
    
    status = rename(g.qfiles->tmp_filename, g.qfiles->q_filename);
  }

 End:
  if(fq != NULL)
    fclose(fq);
  
  if(f != -1)
    close(f);

  return(status);
}

static int read_qinfo(char *qfilename, struct emwin_file_qinfo *qinfo){

  int status = 0;
  int fd = -1;
  int n;

  status = file_exists(qfilename);
  if(status != 0)
    return(status);

  fd = open(qfilename, O_RDONLY);
  if(fd == -1)
    return(-1);

  n = read(fd, qinfo, sizeof(struct emwin_file_qinfo));
  close(fd);

  if(n == -1)
    status = -1;

  return(status);
}

static int write_qinfo(char *qfilename, struct emwin_file_qinfo *qinfo,
		       mode_t mode){
  int status = 0;
  int fd = -1;
  int n;

  fd = open(qfilename, O_CREAT | O_WRONLY | O_TRUNC, mode);
  if(fd == -1)
    return(-1);

  n = write(fd, qinfo, sizeof(struct emwin_file_qinfo));
  close(fd);

  if(n == -1)
    status = -1;

  return(status);
}

int init_emwin_qfiles(void){

  struct emwin_qfiles_st *qfiles = NULL;

  if((qfiles = malloc(sizeof(struct emwin_qfiles_st))) == NULL)
    return(-1);

  /*
   * one '/' between the spooldir and the subdir; then for the file name
   * four more characters for '/', then dot and part number (or 'q'), and NULL.
   * The tmp_filename is the same as the q file, with 'q' -> 't'.
   */
  qfiles->datafn_size = strlen(g.spooldir) + strlen(g.datadir) + 1
    + SEQNUM_SIZE + 4;

  qfiles->tmpfn_size = strlen(g.spooldir) + strlen(g.tmpdir) + 1
    + EMWIN_TOTAL_FNAME_SIZE + 4;

  qfiles->qfn_size = strlen(g.spooldir) + strlen(g.qfilesdir) + 1
    + EMWIN_TOTAL_FNAME_SIZE + 4;

  /*
   * how many digits in the part number? We will leave room for 99999 parts,
   * that is, four more digits.
   */
  qfiles->datafn_size += 4;

  if((qfiles->data_filename = malloc(qfiles->datafn_size)) == NULL){
    free(qfiles);
    return(-1);
  }

  if((qfiles->tmp_filename = malloc(qfiles->tmpfn_size)) == NULL){
    free(qfiles->data_filename);
    free(qfiles);
    return(-1);
  }

  if((qfiles->q_filename = malloc(qfiles->qfn_size)) == NULL){
    free(qfiles->data_filename);
    free(qfiles->tmp_filename);
    free(qfiles);
    return(-1);
  }

  g.qfiles = qfiles;

  return(0);
}

void destroy_emwin_qfiles(void){

  if(g.qfiles == NULL)
    return;

  if(g.qfiles->data_filename != NULL){
    free(g.qfiles->data_filename);
    g.qfiles->data_filename = NULL;
  }

  if(g.qfiles->tmp_filename != NULL){
    free(g.qfiles->tmp_filename);
    g.qfiles->tmp_filename = NULL;
  }

  if(g.qfiles->q_filename != NULL){
    free(g.qfiles->q_filename);
    g.qfiles->q_filename = NULL;
  }

  free(g.qfiles);
  g.qfiles = NULL;
}

static int checksum(struct emwin_packet *ep){

  u_char *b;
  int c = 0;
  int i;

  b = (u_char*)&(ep->data[0]);
  for(i = 0; i < ep->data_size; ++i){
    c += *b;
    ++b;
  }

  if(ep->header.checksum != c)
    return(1);

  /*
  fprintf(stderr, "%s: %d %d\n",ep->header.filename, ep->header.checksum, c);
  */

  return(0);
}

static int checkfilename(struct emwin_packet *ep){

  int status = 0;
  int c;
  int i;
  int namelen = strlen(ep->header.filename);

  for(i = 0; i < EMWIN_FNAME_LEN; ++i){
    c = ep->header.filename[i];
    if(isalnum(c) == 0){
      status = 1;
      break;
    }
  }

  if(status == 0){
    if(ep->header.filename[EMWIN_FNAME_LEN] != '.')
      status = 1;
  }

  if(status == 0){
    for(i = EMWIN_FNAME_LEN + 1; i < namelen; ++i){
      c = ep->header.filename[i];
      if(isalnum(c) == 0){
	status = 1;
	break;
      }
    }
  }

  if(status == 0){
    for(i = 0; i < namelen; ++i){
      c = ep->header.filename[i];
      ep->header.filename[i] = tolower(c);
    }
  }

  return(status);
}

static int is_text_file(char *filename){

  if((filename[9] == 't') || (filename[9] == 'T'))
    return(1);

  return(0);
}

static int try_serverlist(struct emwin_packet *ep,
			  char *bbdata, int bbdata_size){
  /*
   * See if the packet is actually a server list.
   * Some packets contain what seems to be a server list formatted
   * something like
   *
   * /ServerList/<ip:port>|<ip:port>|...|\ServerList\/SatServers/\SatServers\
   *
   * i.e., an empty "SatServers".
   *
   * This function returns 0, or 2 if the packet does not contain a
   * server list.
   */

  /*
   * First unxor the bbdata. Stricly speaking, the serverlist packet
   * is transmitted as a variable length packet and the clients have
   * to detect the end, as we have done (in bb_read_srvlist()).
   */
  memset(ep->rawdata, '\0', EMWIN_PACKET_SIZE);
  memset(ep->bbdata, 0xff, EMWIN_PACKET_SIZE);

  memcpy(ep->bbdata, bbdata, bbdata_size);
  bb_xor(ep->rawdata, bbdata, bbdata_size);
  ep->bbdata_size = bbdata_size;
  ep->rawdata_size = bbdata_size;

  /*
   * Check if it looks like a server list
   */
  if(strncmp(ep->rawdata, "/ServerList/", 12) != 0)
    return(2);

  /*
   * Fill out the entries that will be sent to the
   * client queues.
   */
  memcpy(&ep->queue_data_v1[EMWIN_QUEUE_ENVELOPE_SIZE],
	 ep->bbdata, ep->bbdata_size);
  pack_uint32(ep->queue_data_v1, (uint32_t)ep->bbdata_size, 0);
  ep->queue_data_v1_size = EMWIN_QUEUE_ENVELOPE_SIZE + ep->bbdata_size;

  memcpy(&ep->queue_data_v2[EMWIN_QUEUE_ENVELOPE_SIZE],
	 ep->bbdata, ep->bbdata_size);
  pack_uint32(ep->queue_data_v2, (uint32_t)ep->bbdata_size, 0);
  ep->queue_data_v2_size = EMWIN_QUEUE_ENVELOPE_SIZE + ep->bbdata_size;

  return(0);
}

int save_server_list(struct emwin_packet *ep){

  FILE *f;
  int status = 0;

  if((f = fopen(g.bbserverlist_raw, "w")) == NULL)
    return(-1);

  fprintf(f, "%s", ep->rawdata);
  fclose(f);

  status = write_server_list(ep->rawdata, g.bbserverlist);

  return(status);
}

static int write_server_list(char *s, char *fname){

  int status = 0;
  char *p;
  int c;
  FILE *f;

  f = fopen(fname, "w");
  if(f == NULL)
    return(-1);

  /* Go to the first charracter past the "/ServerList/" word in the list */
  p = &s[12];
  while((*p != '\0') && (status == 0)){
    c = *p++;
    if(c == ':')
      c = ' ';
    else if(c == '|')
      c = '\n';
    else if(c == '\\')
      break;

    if(putc_unlocked(c, f) == EOF)
      status = -1;
  }

  fclose(f);

  return(status);
}
