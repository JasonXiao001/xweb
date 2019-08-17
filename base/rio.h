#ifndef _RIO_H_
#define _RIO_H_

#include <stdlib.h>
#include <unistd.h>

#define RIO_BUFSIZE 8192

typedef struct {
    int rio_fd; // descripter for this internal buf
    int rio_cnt; // unread bytes in internal buf
    char *rio_bufptr; // next unread byte in internal buf
    char rio_buf[RIO_BUFSIZE]; // internal buffer
} rio_t;

/* 
* initialize a rio struct
*/
void rio_init(rio_t *rp, int fd);

/* 
* read n byte direct from fd
*/
ssize_t rio_readn(int fd, void *usrbuf, size_t n);

/* 
* write n byte direct to fd
*/
ssize_t rio_writen(int fd, void *usrbuf, size_t n);

/* 
* read a line from rp with buffer
*/
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);

#endif