#include "cm_prec.h"
#include "cm_dev_account.h"
#include "cm_dev_match.h"
#include "cm_usr_account.h"
#include "cm_usr_match.h"

const char* g_path = NULL;

void stop_server(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp) {
	lh_stop();
}

void server_path(const struct lh_kv_elem *params, const struct lh_kv_elem *cookies, struct lh_response* resp) {
	char buf[512];
	snprintf(buf, sizeof(buf), "{\"path\":\"%d%s\"}", rand()%1000, g_path);
	lh_append_body(resp, buf);
}

int main(int argc, char **argv)
{
	g_path = argv[0];

	lh_register_callback("/stopserver", stop_server);
	lh_register_callback("/serverpath", server_path);
	cm::register_developer_account_cbs();
	cm::register_developer_match_cbs();
	cm::register_user_account_cbs();
	
	lh_start(5555, "../../www");
	lh_loop();
	lh_stop();
	return 0;
}