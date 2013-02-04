#include "cm_account.h"
#include "cm_context.h"
#include "cm_error.h"
#include "cm_util.h"
#include "lh_httpd.h"
#include <postgresql/libpq-fe.h>
#include <uuid/uuid.h>
#include <string.h>

void cmcb_register_developer(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response_body* resp) {
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
	
	//db
    cm_context* pctx = cm_get_context();
    PGconn *accountdb = pctx->accountdb;
	if (PQstatus(accountdb) != CONNECTION_OK) {
        return cm_send_error(resp, CMERR_DB);
    }
    const char *vars[2] = {
        username,
        passwordBase64.str.c_str(),
    };

    PGresult *res = PQexecParams(accountdb,
                                 "INSERT INTO user_account (username, password) VALUES($1, $2)",
                                 2, NULL, vars, NULL, NULL, 0);
    Autofree af_res(res, (Freefunc)PQclear);
    if (PQresultStatus(res) != PGRES_COMMAND_OK){
        cm_send_error(resp, CMERR_EXIST);
        return;
    }
    
    //reply
	lh_append(resp, "{\"error\":0}");
}

void cm_register_account_cbs() {
	lh_register_callback("/developer/register", cmcb_register_developer);
}