#ifndef CM_USR_ACCOUNT_H
#define CM_USR_ACCOUNT_H

struct lh_kv_elem;
struct lh_response;

namespace cm {

//param:
//username:string & password:string & country:int
void cmcb_user_register(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp);

//register
void register_user_account_cbs();

} //namespace cm

#endif // CM_USR_ACCOUNT_H
