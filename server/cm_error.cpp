#include "cm_error.h"
#include "lh_httpd.h"

const char *CMERR_PARAM = "CMERR_PARAM";
const char *CMERR_AUTH = "CMERR_AUTH";
const char *CMERR_DB = "CMERR_DB";
const char *CMERR_EXIST = "CMERR_EXIST";
const char *CMERR_NOT_EXIST = "CMERR_NOT_EXIST";

void cm_send_error(struct lh_response_body* resp, const char *errString) {
	lh_append(resp, errString);
}