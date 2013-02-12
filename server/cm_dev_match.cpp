#include "cm_dev_match.h"
#include "cm_util.h"
#include "cm_context.h"
#include "cm_error.h"
#include "cm_session.h"
#include "lh_httpd.h"
#include <sstream>
#include <uuid/uuid.h>

namespace cm {
	
const uint32_t APP_NUM_LIMIT = 5;
	
void cmdev_list_apps(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp) { 
	//check db
    PGconn *cmatchdb = cm::get_context()->cmatchdb;
    if (PQstatus(cmatchdb) != CONNECTION_OK) {
        return cm_send_error(resp, CMERR_DB);
    }
    
    //check auth
    cm::Session session;
    int err = cm::find_session(cookies, session);
    if (err)
        return cm_send_error(resp, CMERR_AUTH);
    
    //qurey db
    std::stringstream ss_sql;
    ss_sql << "SELECT id, name FROM app WHERE developer_id=" << session.userid;
    PGresult *res = PQexec(cmatchdb, ss_sql.str().c_str());
    Autofree af_res_cl(res, (Freefunc)PQclear);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        return cm_send_error(resp, CMERR_DB);
    }
    
    int rownums = PQntuples(res);
    if (rownums == 0) {
        return cm_send_ok(resp);
    }
    
    std::stringstream ssout;
    ssout << "{\"error\":0,\"apps\":{";
    
    bool first = true;
    for (int i = 0; i < rownums; ++i) {
        const char* sid = PQgetvalue(res, i, 0);
        const char* sname = PQgetvalue(res, i, 1);
        if (sid && sname) {
            if (first) {
                first = false;
            } else {
                ssout << ",";
            }
            ssout << "\"" << sid << "\"" << ":\"" << sname << "\"";
        }
    }
    ssout << "}}";
    
    //send to client
	lh_append_body(resp, ssout.str().c_str());
}

void cmdev_add_app(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp) {
	//check db
    PGconn *cmatchdb = cm::get_context()->cmatchdb;
    if (PQstatus(cmatchdb) != CONNECTION_OK) {
        return cm_send_error(resp, CMERR_DB);
    }
    
    //check auth
    cm::Session session;
    int err = cm::find_session(cookies, session);
    if (err) {
        return cm_send_error(resp, CMERR_AUTH);
	}
    
    //parse param
	const char *appname = lh_kv_string(params, "appname", &err);
    if (err) {
        return cm_send_error(resp, CMERR_PARAM);
	}
    
    //secret key
	uuid_t uuid;
    uuid_generate(uuid);
	Base64 secret64((const char*)uuid, sizeof(uuid));
    
    //count limit
    std::stringstream ss_sql;
    ss_sql << "SELECT COUNT(0) FROM app WHERE developer_id=" << session.userid << ";";
    PGresult *res = PQexec(cmatchdb, ss_sql.str().c_str());
    Autofree af_res_cl(res, (Freefunc)PQclear);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        return cm_send_error(resp, CMERR_DB);
    }
    const char* sappnum = PQgetvalue(res, 0, 0);
    if (sappnum == NULL) {
        return cm_send_error(resp, CMERR_DB);
    }
    uint32_t appnum = s2int32(sappnum, &err);
    if (err){
        return cm_send_error(resp, CMERR_DB);
    }
    if (appnum >= APP_NUM_LIMIT) {
        return cm_send_error(resp, CMERR_OUT_OF_RANGE);
    }
    
    //insert into db
    const char *vars[3] = {
        appname,
        secret64.str.c_str(),
        session.userid.c_str(),
    };
    res = PQexecParams(cmatchdb,
                       "INSERT INTO app (name, secret, developer_id) VALUES($1, $2, $3) RETURNING id;",
                       3, NULL, vars, NULL, NULL, 0);
    Autofree af_res_insert(res, (Freefunc)PQclear);
    if (PQresultStatus(res) != PGRES_TUPLES_OK){
        return cm_send_error(resp, CMERR_DB);
    }

    //get app id
    const char* appid = PQgetvalue(res, 0, 0);
    if (appid == NULL) {
        return cm_send_error(resp, CMERR_DB);
    }
    
    //send to client
	std::stringstream ss;
	ss << "{\"error\":0, \"id\":" << appid << "}";
	lh_append_body(resp, ss.str().c_str());
}

void register_developer_match_cbs() {
	lh_register_callback("/cmapi/developer/listapps", cmdev_list_apps);
	lh_register_callback("/cmapi/developer/addapp", cmdev_add_app);
}

} //namespace cm