#ifndef CM_DEV_ACCOUNT_H
#define CM_DEV_ACCOUNT_H

struct lh_kv_elem;
struct lh_response;

namespace cm {

//param:
//username:string
//password:string
void cmcb_developer_register(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp);

//param:
//username:string
//password:string
void cmcb_developer_login(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp);

//param:
//usertoken:cookies
void cmcb_developer_relogin(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp);

//param:
//usertoken:cookies
void cmcb_developer_logout(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp);


void register_developer_account_cbs();

} //namespace cm

#endif // CM_DEV_ACCOUNT_H
