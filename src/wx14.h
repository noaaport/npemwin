/*
 * $Id$
 */
#ifndef WX14_H
#define WX14_H

/* non-os errors */
#define WX14_ERROR_INVALID_HEADER 1    /* error in bytes 0,1 */
#define WX14_ERROR_INVALID_CMDTYPE 2   /* unrecognized command type */
#define WX14_ERROR_UNEXPECTED_CMDTYPE 3

#define WX14_ERROR_READ_TIMEOUT 4 /* time out (poll) before reading anything */
#define WX14_ERROR_READ_EOF 5     /* eof (server disconnected) */
#define WX14_ERROR_READ_SHORT 6   /* short read (timedout or disconnect)
				   * while reading */

#define WX14_ERROR_EMWIN_BUF 7    /* emwin packet buffer size too small */
#define WX14_ERROR_EMWIN_FILL_PACKET 8  /* error from fill_packet_struct_wc14
					 * function */

int wx14_read_emwin_packet(int fd, unsigned int secs, int retry,
			   void *buf, size_t *size);

#endif
