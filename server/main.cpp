#include "lh_httpd.h"
#include "cm_account.h"
#include <stdio.h>
#include <stdlib.h>

const char* g_path = NULL;

void stop_server(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response_body* resb_body) {
	lh_stop();
}

void server_path(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response_body* resb_body) {
	char buf[512];
	snprintf(buf, sizeof(buf), "{\"path\":\"%d%s\"}", rand()%1000, g_path);
	lh_append(resb_body, buf);
}

int main(int argc, char **argv)
{
	g_path = argv[0];

	lh_register_callback("/stopserver", stop_server);
	lh_register_callback("/serverpath", server_path);
	cm_register_account_cbs();
	
	lh_start(5555, "../../www");
	lh_loop();
	lh_stop();
	return 0;
}