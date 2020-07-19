#ifndef SOCK_H
#define SOCK_H

#include <inttypes.h>
#include "ib.h"

#define SOCK_SYNC_MSG  "sync"

ssize_t
sock_read (int, void*, size_t);

ssize_t
sock_write (int, void*, size_t);

int
sock_create_bind (char*);

int
sock_create_connect (char*, char*);

int
sock_set_qp_info(int, struct QPInfo*);

int
sock_get_qp_info(int, struct QPInfo*);

#endif
