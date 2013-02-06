#include "cm_account.h"
#include "cm_context.h"
#include "cm_error.h"
#include "cm_util.h"
#include "lh_httpd.h"
#include <postgresql/libpq-fe.h>
#include <uuid/uuid.h>
#include <string.h>

static const size_t NAME_LEN_MAX = 40;

void cmcb_developer_register(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp) {
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
    Sha1 passwordSh1(password, strlen(password));
	Base64 passwordBase64(passwordSh1.buf, sizeof(passwordSh1.buf));
	
	//db
    PGconn *accountdb = cm_get_context()->accountdb;
    const char *vars[2] = {
        username,
        passwordBase64.str.c_str(),
    };
    PGresult *res = PQexecParams(accountdb,
                                 "INSERT INTO developer_account (username, password) VALUES($1, $2)",
                                 2, NULL, vars, NULL, NULL, 0);
    Autofree af_res(res, (Freefunc)PQclear);
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
    if (err) {
        return cm_send_error(resp, CMERR_PARAM);
    }
	
	//encrypt password
    Sha1 passwordSh1(password, strlen(password));
	Base64 passwordBase64(passwordSh1.buf, sizeof(passwordSh1.buf));
	
	//sql select
    PGconn *acdb = cm_get_context()->accountdb;
    const char *vars[1] = { 
        username
    };
    PGresult *res = PQexecParams(acdb,
                                 "SELECT id, password FROM user_account WHERE username=$1",
                                 1, NULL, vars, NULL, NULL, 0);
    Autofree af_res(res, (Freefunc)PQclear);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        return cm_send_error(resp, CMERR_DB);
    }
    
    int rownums = PQntuples(res);
    if (rownums == 0) {
        return cm_send_error(resp, CMERR_NOT_EXIST);
    }
    
    //fetch pq value
    const char* pq_userid = PQgetvalue(res, 0, 0);
	const char* pq_password = PQgetvalue(res, 0, 1);
    if (!pq_userid || !pq_password){
        return cm_send_error(resp, CMERR_DB);
    }
    
    //password compare
    if (strcmp(pq_password, passwordBase64.str.c_str()) != 0) {
        return cm_send_error(resp, CMERR_AUTH);
    }
    
    //session
	err = 0;
    //const char *oldtoken = lh_kv_string(cookies, "usertoken", &err);
	
//    cm_session session;
//    session.userid = struserid;
//    session.username = ue_username;
//    std::string usertoken;
//    if (oldtoken) {
//        delSession(oldtoken);
//    }
//    err = newSession(usertoken, session);
//    if (err) {
//        return cm_send_error(err_redis, req);
//    }
	
	//reply
	lh_append_body(resp, "{\"error\":0}");
}

void cm_register_account_cbs() {
	lh_register_callback("/developer/register", cmcb_developer_register);
	lh_register_callback("/developer/login", cmcb_developer_login);	
}