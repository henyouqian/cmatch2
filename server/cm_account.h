#ifndef CM_ACCOUNT_H
#define CM_ACCOUNT_H

void cmcb_register_developer(const struct lh_kv_elem*, const struct lh_kv_elem*, struct lh_response_body*);

void cm_register_account_cbs();

#endif // CM_ACCOUNT_H
