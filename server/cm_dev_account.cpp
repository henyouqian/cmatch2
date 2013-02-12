#include "cm_dev_account.h"
#include "cm_context.h"
#include "cm_error.h"
#include "cm_util.h"
#include "lh_httpd.h"
#include "cm_session.h"
#include <postgresql/libpq-fe.h>
#include <string.h>

namespace cm {

static const size_t NAME_LEN_MAX = 40;

void cmcb_developer_register(const lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp) {
	//parse param
    int err = 0;
	const char *username = lh_kv_string(params, "username", &err);
    const char *password = lh_kv_string(params, "password", &err);
    if (err) {
        return cm_send_error(resp, CMERR_PARAM);
    }
	if (strlen(username) > NAME_LEN_MAX) {
		return cm_send_error(resp, CMERR_OUT_OF_RANGE);
	}
	
	//encrypt password
    cm::Sha1 passwordSh1(password, strlen(password));
	cm::Base64 passwordBase64(passwordSh1.buf, sizeof(passwordSh1.buf));
	
	//db
    PGconn *accountdb = cm::get_context()->accountdb;
    const char *vars[2] = {
        username,
        passwordBase64.str.c_str(),
    };
    PGresult *res = PQexecParams(accountdb,
                                 "INSERT INTO developer_account (username, password) VALUES($1, $2)",
                                 2, NULL, vars, NULL, NULL, 0);
    cm::Autofree af_res(res, (cm::Freefunc)PQclear);
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
        return cm_send_error(resp, CMERR_DB);
    
    //reply
	lh_append_body(resp, "{\"error\":0}");
}

void cmcb_developer_login(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp) {
	//parse param
    int err = 0;
	const char *username = lh_kv_string(params, "username", &err);
    const char *password = lh_kv_string(params, "password", &err);
    if (err)
        return cm_send_error(resp, CMERR_PARAM);
	
	//encrypt password
    cm::Sha1 passwordSh1(password, strlen(password));
	cm::Base64 passwordBase64(passwordSh1.buf, sizeof(passwordSh1.buf));
	
	//sql select
    PGconn *acdb = cm::get_context()->accountdb;
    const char *vars[1] = { 
        username
    };
    PGresult *res = PQexecParams(acdb,
                                 "SELECT id, password FROM developer_account WHERE username=$1",
                                 1, NULL, vars, NULL, NULL, 0);
    cm::Autofree af_res(res, (cm::Freefunc)PQclear);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
        return cm_send_error(resp, CMERR_DB);
    
    int rownums = PQntuples(res);
    if (rownums == 0)
        return cm_send_error(resp, CMERR_NOT_EXIST);
    
    //fetch pq value
    const char* pq_userid = PQgetvalue(res, 0, 0);
	const char* pq_password = PQgetvalue(res, 0, 1);
    if (!pq_userid || !pq_password)
        return cm_send_error(resp, CMERR_DB);
    
    //password compare
    if (strcmp(pq_password, passwordBase64.str.c_str()) != 0)
        return cm_send_error(resp, CMERR_AUTH);
    
    //session
    const char *oldtoken = lh_kv_string(cookies, "usertoken", NULL);
	if (oldtoken)
		del_session(oldtoken);
	std::string token;
    err = new_session(token, pq_userid, username);
    if (err)
        return cm_send_error(resp, CMERR_REDIS);
		
	//cookies
	char buf[512];
	snprintf(buf, sizeof(buf), "usertoken=%s; path=/", token.c_str());
	lh_append_header(resp, "Set-Cookie", buf);
	snprintf(buf, sizeof(buf), "username=%s; path=/", username);
	lh_append_header(resp, "Set-Cookie", buf);
	
	//reply
	lh_append_body(resp, "{\"error\":0}");
}

void cmcb_developer_relogin(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp){
	//parse cookies
	int err = 0;
	const char *usertoken = lh_kv_string(cookies, "usertoken", &err);
	if (err)
		return cm_send_error(resp, CMERR_PARAM);
	
	//find session and renew
	Session session;
    err = cm::find_session(usertoken, session);
    if (err)
        return cm_send_error(resp, CMERR_AUTH);
    
    del_session(usertoken);
    std::string newtoken;
    new_session(newtoken, session.userid.c_str(), session.username.c_str());
    
    //cookies
	char buf[512];
	snprintf(buf, sizeof(buf), "usertoken=%s; path=/", newtoken.c_str());
	lh_append_header(resp, "Set-Cookie", buf);
    
	//reply
	lh_append_body(resp, "{\"error\":0}");
}

void cmcb_developer_logout(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp){
	//parse cookies
	int err = 0;
	const char *usertoken = lh_kv_string(cookies, "usertoken", &err);
	if (err)
		return cm_send_error(resp, CMERR_PARAM);

	//delete usertoken
	del_session(usertoken);

	//reply
	lh_append_body(resp, "{\"error\":0}");
}

void register_developer_account_cbs() {
	lh_register_callback("/cmapi/developer/register", cmcb_developer_register);
	lh_register_callback("/cmapi/developer/login", cmcb_developer_login);
	lh_register_callback("/cmapi/developer/relogin", cmcb_developer_relogin);
	lh_register_callback("/cmapi/developer/logout", cmcb_developer_logout);
}

} //namespace cm