#include "cm_prec.h"
#include "cm_usr_account.h"

namespace cm {

void cmcb_user_register(const lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp) {
	//parse param
    int err = 0;
	const char *username = lh_kv_string(params, "username", &err);
    const char *password = lh_kv_string(params, "password", &err);
	const char *country = lh_kv_string(params, "country", &err);
    if (err) {
        return cm_send_error(resp, CMERR_PARAM);
    }
	
	//encrypt password
    cm::Sha1 passwordSh1(password, strlen(password));
	cm::Base64 passwordBase64(passwordSh1.buf, sizeof(passwordSh1.buf));
	
	//db
    PGconn *accountdb = cm::get_context()->accountdb;
	const int VAR_NUMS = 3;
    const char *vars[VAR_NUMS] = {
        username,
        passwordBase64.str.c_str(),
		country,
    };
	PGresult *res = PQexecParams(accountdb,
								 "INSERT INTO user_account (username, password, country) VALUES($1, $2, $3)",
								 VAR_NUMS, NULL, vars, NULL, NULL, 0);
    cm::Autofree af_res(res, (cm::Freefunc)PQclear);
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
        return cm_send_error(resp, CMERR_DB);
    
    //reply
	lh_append_body(resp, "{\"error\":0}");
}

void register_user_account_cbs() {
	lh_register_callback("/cmapi/user/register", cmcb_user_register);
}

} //namespace cm