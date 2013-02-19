#include "cm_prec.h"
#include "cm_session.h"

namespace cm {
	
static const time_t SESSION_LIFE_SEC = 60*60; //one hour
	
int new_session(std::string &outtoken, const char *userid, const char *username) {
	assert(userid && username);
		
	uuid_t uuid;
    uuid_generate(uuid);
	Base64 tokenB64((const char*)uuid, sizeof(uuid));
    outtoken = tokenB64.str;
    
    redisContext *redis = get_context()->redis;
    char buf[512];
    MemIO mio(buf, sizeof(buf));
    mio.writeString(userid);
    mio.writeString(username);
    redisReply *reply = (redisReply*)redisCommand(redis, "SETEX session:%s %u %b",
                                     outtoken.c_str(), SESSION_LIFE_SEC, mio.p0, mio.length());
    Autofree _af(reply, freeReplyObject);
    if (!reply)
        return -1;
    return 0;
}

void del_session(const char* token) {
    redisContext *redis = cm::get_context()->redis;
    redisReply *reply = (redisReply*)redisCommand(redis, "DEL session:%s", token);
    Autofree _af(reply, freeReplyObject);
}

int find_session(const char *token, Session &session) {
    redisContext *redis = cm::get_context()->redis;
    redisReply *reply = (redisReply*)redisCommand(redis, "GET session:%s", token);
    Autofree _af(reply, freeReplyObject);
    if (reply && reply->type == REDIS_REPLY_STRING ) {
        MemIO mio(reply->str, reply->len);
        const char *userid = mio.readString();
        const char *username = mio.readString();
        if (userid && username) {
            session.userid = userid;
            session.username = username;
			return 0;
        }
	}
    return -1;
}

int find_session(const struct lh_kv_elem *cookies, Session &session) {
	if (!cookies)
		return -1;
	int err = 0;
	const char *usertoken = lh_kv_string(cookies, "usertoken", &err);
	if (err)
		return -1;
		
	return find_session(usertoken, session);
}

} //namespace cm