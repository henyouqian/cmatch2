#include "cm_error.h"
#include "lh_httpd.h"
#include <assert.h>
#include <stdio.h>

const char *CMERR_PARAM = "CMERR_PARAM";
const char *CMERR_AUTH = "CMERR_AUTH";
const char *CMERR_DB = "CMERR_DB";
const char *CMERR_EXIST = "CMERR_EXIST";
const char *CMERR_NOT_EXIST = "CMERR_NOT_EXIST";
const char *CMERR_OUT_OF_RANGE = "CMERR_OUT_OF_RANGE";
const char *CMERR_REDIS = "CMERR_REDIS";

void cm_send_error(struct lh_response* resp, const char *errString) {
	assert(resp && errString);
	char buf[512];
	snprintf(buf, sizeof(buf), "{\"error\":\"%s\"}", errString);
	lh_append_body(resp, buf);
}

void cm_send_ok(struct lh_response* resp) {
	assert(resp);
	lh_append_body(resp, "{\"error\":0}");
}