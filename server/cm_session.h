#ifndef CM_SESSION_H
#define CM_SESSION_H

#include <string>

struct lh_kv_elem;

namespace cm {

struct Session {
    std::string userid;
    std::string username;
};

int new_session(std::string &newtoken, const char *userid, const char *username);
void del_session(const char *token);
int find_session(const char *token, Session &session);
int find_session(const struct lh_kv_elem *cookies, Session &session);

	
} //namespace cm

#endif // CM_SESSION_H
