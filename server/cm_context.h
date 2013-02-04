#ifndef CM_CONTEXT_H
#define CM_CONTEXT_H

#include <postgresql/libpq-fe.h>
#include <hiredis/hiredis.h>
#include <pthread.h>

struct cm_context {
    PGconn *accountdb;
    PGconn *cmatchdb;
    redisContext *redis;
    pthread_t tid;
};

cm_context *cm_get_context();

#endif // CM_CONTEXT_H
