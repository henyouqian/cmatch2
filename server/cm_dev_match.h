#ifndef CM_DEV_MATCH_H
#define CM_DEV_MATCH_H

struct lh_kv_elem;
struct lh_response;

namespace cm {
	
//@param
//no param
void cmdev_list_apps(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp);

//@param
//appname:string
void cmdev_add_app(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp);

//@param
//appid:id & appname:string
void cmdev_edit_app(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp);

//@param
//appid:id
void cmdev_app_secret(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp);

//@param
//appid:id
void cmdev_list_games(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp);

//@param
//appid:id & gamename:string
void cmdev_add_game(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp);

//@param
//gameid:id & gamename:string
void cmdev_edit_game(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp);

//rigester
void register_developer_match_cbs();

} //namespace cm

#endif // CM_DEV_MATCH_H
