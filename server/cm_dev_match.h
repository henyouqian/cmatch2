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

void register_developer_match_cbs();

} //namespace cm

#endif // CM_DEV_MATCH_H
