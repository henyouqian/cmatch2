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
const uint32_t GAME_NUM_LIMIT = 20;
const uint32_t MATCH_LIST_MAX = 50+1;
	
void cmdev_list_app(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp) { 
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
	lh_appendf_body(resp, "{\"error\":0, \"appid\":%s}", appid);
}

void cmdev_edit_app(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp) {
    //parse param
    int err = 0;
    const char *appid = lh_kv_string(params, "appid", &err);
    const char *appname = lh_kv_string(params, "appname", &err);
    if (err) {
        return cm_send_error(resp, CMERR_PARAM);
    }
    
    //check auth
    cm::Session session;
    err = cm::find_session(cookies, session);
    if (err) {
        return cm_send_error(resp, CMERR_AUTH);
	}
    
    //update db
	PGconn *cmatchdb = cm::get_context()->cmatchdb;
    if (PQstatus(cmatchdb) != CONNECTION_OK) {
        return cm_send_error(resp, CMERR_DB);
    }
    const int VAR_NUMS = 3;
    const char *vars[VAR_NUMS] = {
        appname,
        appid,
        session.userid.c_str(),
    };
    PGresult *res = PQexecParams(cmatchdb,
                       "UPDATE app SET name=$1 WHERE id=$2 AND developer_id=$3",
                       VAR_NUMS, NULL, vars, NULL, NULL, 0);
    Autofree af_res_insert(res, (Freefunc)PQclear);
    if (PQresultStatus(res) != PGRES_COMMAND_OK){
        return cm_send_error(resp, CMERR_DB);
    }
    
    const char *affected = PQcmdTuples(res);
    if (!affected || *affected == '0') {
        return cm_send_error(resp, CMERR_NOT_EXIST);
    }
    
    //send to client
	lh_append_body(resp, "{\"error\":0}");
}

void cmdev_app_secret(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp) {
    //parse param
    int err = 0;
	const char *appid = lh_kv_string(params, "appid", &err);
    if (err) {
        return cm_send_error(resp, CMERR_PARAM);
	}
    
    //check auth
    cm::Session session;
    err = cm::find_session(cookies, session);
    if (err) {
        return cm_send_error(resp, CMERR_AUTH);
	}
    
    //select from db
	PGconn *cmatchdb = cm::get_context()->cmatchdb;
    if (PQstatus(cmatchdb) != CONNECTION_OK) {
        return cm_send_error(resp, CMERR_DB);
    }
	
	const int VAR_NUMS = 2;
	const char *vars[VAR_NUMS] = {
		session.userid.c_str(),
        appid
    };
    PGresult *res = PQexecParams(cmatchdb,
                       "SELECT secret FROM app WHERE developer_id=$1 AND id=$2;",
                       VAR_NUMS, NULL, vars, NULL, NULL, 0);
    Autofree af_res_cl(res, (Freefunc)PQclear);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        return cm_send_error(resp, CMERR_DB);
    }
    
    int rows = PQntuples(res);
    if (rows != 1) {
        return cm_send_error(resp, CMERR_NOT_EXIST);
    }
    const char* secret = PQgetvalue(res, 0, 0);
    if (secret == NULL) {
        return cm_send_error(resp, CMERR_DB);
    }
    
    //send to client
	lh_appendf_body(resp, "{\"error\":0, \"secret\":\"%s\"}", secret);
}

void cmdev_list_game(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp) {
    //check auth
    cm::Session session;
    int err = cm::find_session(cookies, session);
    if (err) {
        return cm_send_error(resp, CMERR_AUTH);
	}
	
	//check param
	const char *appid = lh_kv_string(params, "appid", &err);
    if (err) {
        return cm_send_error(resp, CMERR_PARAM);
	}
	
	//check db
    PGconn *cmatchdb = cm::get_context()->cmatchdb;
    if (PQstatus(cmatchdb) != CONNECTION_OK) {
        return cm_send_error(resp, CMERR_DB);
    }
	
    //qurey db
	const int VAR_NUMS = 1;
	const char *vars[VAR_NUMS] = {
		appid,
    };
    PGresult *res = PQexecParams(cmatchdb,
                       "SELECT id, name FROM game WHERE app_id=$1;",
                       VAR_NUMS, NULL, vars, NULL, NULL, 0);
    Autofree af_res_cl(res, (Freefunc)PQclear);
	
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        return cm_send_error(resp, CMERR_DB);
    }
    
    int rownums = PQntuples(res);
    if (rownums == 0) {
        return cm_send_ok(resp);
    }
    
    std::stringstream ssout;
    ssout << "{\"error\":0,\"games\":{";
    
    bool first = true;
    for (int i = 0; i < rownums; ++i) {
        const char* sid = PQgetvalue(res, i, 0);
        const char* sname = PQgetvalue(res, i, 1);
        if (sid && sname) {
            if (first)
                first = false;
            else
                ssout << ",";
            ssout << "\"" << sid << "\"" << ":\"" << sname << "\"";
        }
    }
    ssout << "}}";
    
    //send to client
	lh_append_body(resp, ssout.str().c_str());
}

void cmdev_add_game(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp) {
    //check auth
    cm::Session session;
    int err = cm::find_session(cookies, session);
    if (err) {
        return cm_send_error(resp, CMERR_AUTH);
	}
    
    //parse param
	const char *appid = lh_kv_string(params, "appid", &err);
	const char *gamename = lh_kv_string(params, "gamename", &err);
    if (err) {
        return cm_send_error(resp, CMERR_PARAM);
	}

	//check db
    PGconn *cmatchdb = cm::get_context()->cmatchdb;
    if (PQstatus(cmatchdb) != CONNECTION_OK) {
        return cm_send_error(resp, CMERR_DB);
    }
	
    //count limit
	const int VAR_NUMS = 1;
	const char *vars[VAR_NUMS] = {
		appid,
    };
    PGresult *res = PQexecParams(cmatchdb,
                       "SELECT COUNT(0) FROM game WHERE app_id=$1;",
                       VAR_NUMS, NULL, vars, NULL, NULL, 0);
    Autofree af_res_cl(res, (Freefunc)PQclear);
	
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        return cm_send_error(resp, CMERR_DB);
    }
	
    const char* s_gamenums = PQgetvalue(res, 0, 0);
    if (s_gamenums == NULL) {
        return cm_send_error(resp, CMERR_DB);
    }
    uint32_t gamenums = s2int32(s_gamenums, &err);
    if (err){
        return cm_send_error(resp, CMERR_DB);
    }
    if (gamenums >= GAME_NUM_LIMIT) {
        return cm_send_error(resp, CMERR_OUT_OF_RANGE);
    }
	
    //insert into db
	const int VAR_NUMS2 = 3;
    const char *vars2[VAR_NUMS2] = {
        gamename,
        appid,
		session.userid.c_str(),
    };
	res = PQexecParams(	cmatchdb,
						"INSERT INTO game (name, app_id, developer_id) "
						"SELECT $1, $2, $3 WHERE $3=(SELECT developer_id FROM app WHERE id=$2) "
						"RETURNING id;",
						VAR_NUMS2, NULL, vars2, NULL, NULL, 0);
    Autofree af_res_insert(res, (Freefunc)PQclear);
    if (PQresultStatus(res) != PGRES_TUPLES_OK){
        return cm_send_error(resp, CMERR_DB);
    }

    //get game id
    const char* gameid = PQgetvalue(res, 0, 0);
    if (gameid == NULL) {
        return cm_send_error(resp, CMERR_DB);
    }
    
    //send to client
	lh_appendf_body(resp, "{\"error\":0, \"gameid\":%s}", gameid);
}

void cmdev_edit_game(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp) {
    //parse param
    int err = 0;
    const char *gameid = lh_kv_string(params, "gameid", &err);
    const char *gamename = lh_kv_string(params, "gamename", &err);
    if (err) {
        return cm_send_error(resp, CMERR_PARAM);
    }
    
    //check auth
    cm::Session session;
    err = cm::find_session(cookies, session);
    if (err) {
        return cm_send_error(resp, CMERR_AUTH);
	}
    
    //update db
	PGconn *cmatchdb = cm::get_context()->cmatchdb;
    if (PQstatus(cmatchdb) != CONNECTION_OK) {
        return cm_send_error(resp, CMERR_DB);
    }
    const int VAR_NUMS = 3;
    const char *vars[VAR_NUMS] = {
        gamename,
        gameid,
		session.userid.c_str(),
    };
    PGresult *res = PQexecParams(cmatchdb,
                       "UPDATE game SET name=$1 WHERE id=$2 AND developer_id=$3;",
                       VAR_NUMS, NULL, vars, NULL, NULL, 0);
    Autofree af_res_insert(res, (Freefunc)PQclear);
    if (PQresultStatus(res) != PGRES_COMMAND_OK){
        return cm_send_error(resp, CMERR_DB);
    }
    
    const char *affected = PQcmdTuples(res);
    if (!affected || *affected == '0') {
        return cm_send_error(resp, CMERR_NOT_EXIST);
    }
    
    //send to client
	lh_append_body(resp, "{\"error\":0}");
}

void cmdev_list_match(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp) {
    //check auth
    cm::Session session;
    int err = cm::find_session(cookies, session);
    if (err) {
        return cm_send_error(resp, CMERR_AUTH);
	}
	
	//check param
	const char *appid = lh_kv_string(params, "appid", &err);
	const char *offset = lh_kv_string(params, "offset", &err);
	uint32_t limit = lh_kv_uint32(params, "limit", &err);
    if (err) {
        return cm_send_error(resp, CMERR_PARAM);
	}
	limit = std::min(MATCH_LIST_MAX, limit);
	
	//check db
    PGconn *cmatchdb = cm::get_context()->cmatchdb;
    if (PQstatus(cmatchdb) != CONNECTION_OK) {
        return cm_send_error(resp, CMERR_DB);
    }
	
    //qurey db
	std::stringstream ss_limit;
	ss_limit << limit;
	const int VAR_NUMS = 3;
	const char *vars[VAR_NUMS] = {
		appid,
		offset,
		ss_limit.str().c_str(),
    };
    PGresult *res = PQexecParams(cmatchdb,
						"SELECT id, name, begin_time, end_time, "
							"(SELECT name FROM game WHERE id=game_id) "
							"FROM match WHERE app_id=$1 "
							"ORDER BY id DESC OFFSET $2 LIMIT $3;",
						VAR_NUMS, NULL, vars, NULL, NULL, 0);
    Autofree af_res_cl(res, (Freefunc)PQclear);
	
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        return cm_send_error(resp, CMERR_DB);
    }
    
    int rownums = PQntuples(res);
    if (rownums == 0) {
        return cm_send_ok(resp);
    }
    
    std::stringstream ssout;
    ssout << "{\"error\":0,\"matches\":[";
    
    bool first = true;
    for (int i = 0; i < rownums; ++i) {
        const char* id = PQgetvalue(res, i, 0);
        const char* name = PQgetvalue(res, i, 1);
		const char* begintime = PQgetvalue(res, i, 2);
		const char* endtime = PQgetvalue(res, i, 3);
		const char* gamename = PQgetvalue(res, i, 4);
        if (id && name && begintime && endtime && gamename) {
            if (first)
                first = false;
            else
                ssout << ",";
            ssout << "[" << id << ",\"" << name << "\",\"" << gamename << "\",\"" 
				<< begintime << "\",\"" << endtime << "\"]";
        }
    }
    ssout << "]}";
    
    //send to client
	lh_append_body(resp, ssout.str().c_str());
}

void cmdev_add_match(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp) {
    //check auth
    cm::Session session;
    int err = cm::find_session(cookies, session);
    if (err) {
        return cm_send_error(resp, CMERR_AUTH);
	}
    
    //parse param
	const char *appid = lh_kv_string(params, "appid", &err);
	const char *gameid = lh_kv_string(params, "gameid", &err);
	const char *begin = lh_kv_string(params, "begin", &err);
	const char *end = lh_kv_string(params, "end", &err);
	const char *matchname = lh_kv_string(params, "matchname", NULL);
	if (matchname && matchname[0] == 0) {
		matchname = NULL;
	}
    if (err) {
        return cm_send_error(resp, CMERR_PARAM);
	}

	//check db
    PGconn *cmatchdb = cm::get_context()->cmatchdb;
    if (PQstatus(cmatchdb) != CONNECTION_OK) {
        return cm_send_error(resp, CMERR_DB);
    }
	
    //insert into db
	const int VAR_NUMS = 6;
    const char *vars[VAR_NUMS] = {
		matchname,
		gameid,
		appid,
		session.userid.c_str(),
		begin,
		end,
    };
	PGresult *res = PQexecParams(cmatchdb,
								 "INSERT INTO match (name, game_id, app_id, developer_id, begin_time, end_time) "
								 "SELECT $1, $2, $3, $4, $5::TIMESTAMP, $6::TIMESTAMP "
									"WHERE $4=(SELECT developer_id FROM game WHERE id=$2) "
										"AND $3=(SELECT app_id FROM game WHERE id=$2) "
										"AND $5::TIMESTAMP<$6::TIMESTAMP "
								 "RETURNING id;",
								 VAR_NUMS, NULL, vars, NULL, NULL, 0);
    Autofree af_res_insert(res, (Freefunc)PQclear);
    if (PQresultStatus(res) != PGRES_TUPLES_OK){
        return cm_send_error(resp, CMERR_DB);
    }

    //get match id
    const char* matchid = PQgetvalue(res, 0, 0);
    if (matchid == NULL) {
        return cm_send_error(resp, CMERR_DB);
    }
    
    //send to client
	lh_appendf_body(resp, "{\"error\":0, \"matchid\":%s}", matchid);
}

void register_developer_match_cbs() {
	lh_register_callback("/cmapi/developer/listapp", cmdev_list_app);
	lh_register_callback("/cmapi/developer/addapp", cmdev_add_app);
	lh_register_callback("/cmapi/developer/editapp", cmdev_edit_app);
	lh_register_callback("/cmapi/developer/appsecret", cmdev_app_secret);
	
	lh_register_callback("/cmapi/developer/listgame", cmdev_list_game);
	lh_register_callback("/cmapi/developer/addgame", cmdev_add_game);
	lh_register_callback("/cmapi/developer/editgame", cmdev_edit_game);
	
	lh_register_callback("/cmapi/developer/listmatch", cmdev_list_match);
	lh_register_callback("/cmapi/developer/addmatch", cmdev_add_match);
}

} //namespace cm