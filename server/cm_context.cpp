#include "cm_context.h"
#include <assert.h>
#include <unistd.h>
#include <signal.h>

namespace cm {

static context _context;

static void *check_thread(void* arg) {
	int n = 0;
    while(1) {
        sleep(5);
		n += 5;
		if ( n > 60 ) {
			n = 0;
			redisReply *reply = (redisReply*)redisCommand(_context.redis, "TTL ___");
			if (reply)
				freeReplyObject(reply);
		}
        if (_context.redis->err) {
            _context.redis = redisConnect("127.0.0.1", 6379);
        }
        if (PQstatus(_context.accountdb) != CONNECTION_OK) {
            _context.cmatchdb = PQsetdbLogin("127.0.0.1","5432","","","account_db","postgres","nmmgbnmmgb");
        }
        if (PQstatus(_context.cmatchdb) != CONNECTION_OK) {
            _context.cmatchdb = PQsetdbLogin("127.0.0.1","5432","","","cmatch_db","postgres","nmmgbnmmgb");
        }
    }
    
    return NULL;
}

namespace {
    class ContextLife {
    public:
        ContextLife() {
            //pq connection
            _context.accountdb = PQsetdbLogin("127.0.0.1","5432","","","account_db","postgres","Nmmgb808313");
            if (PQstatus(_context.accountdb) != CONNECTION_OK) {
                fprintf(stderr, "Connection to account_db failed: %s",
                        PQerrorMessage(_context.accountdb));
                assert(0);
            }
            _context.cmatchdb = PQsetdbLogin("127.0.0.1","5432","","","cmatch_db","postgres","Nmmgb808313");
            if (PQstatus(_context.cmatchdb) != CONNECTION_OK) {
                fprintf(stderr, "Connection to cmatch_db failed: %s",
                        PQerrorMessage(_context.cmatchdb));
                assert(0);
            }
            
            //redis
            _context.redis = redisConnect("127.0.0.1", 6379);
            
            //check thread
            pthread_create(&(_context.tid), NULL, check_thread, NULL);
        }
        ~ContextLife() {
            PQfinish(_context.accountdb);
            PQfinish(_context.cmatchdb);
            redisFree(_context.redis);
			pthread_kill(_context.tid, SIGKILL);
        }
    };
    
    ContextLife _contextlife;
}

context *get_context() {
    return &_context;
}

}