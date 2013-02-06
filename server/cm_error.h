#ifndef CM_ERROR_H
#define CM_ERROR_H

extern const char *CMERR_PARAM;
extern const char *CMERR_AUTH;
extern const char *CMERR_DB;
extern const char *CMERR_EXIST;
extern const char *CMERR_NOT_EXIST;
extern const char *CMERR_OUT_OF_RANGE;

struct lh_response_body;

void cm_send_error(struct lh_response* resp, const char *errString);

#endif // CM_ERROR_H
