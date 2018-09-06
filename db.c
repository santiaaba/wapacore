#include "db.h"

void db_init(T_db *db){
	db->con = mysql_init(NULL);
}

int db_connect(T_db *db, T_config *c){
	if (!mysql_real_connect(db->con, c->db_server,
		c->db_user, c->db_pass, c->db_name, 0, NULL, 0)) {
		return 0;
	}
	return 1;
}

void db_close(T_db *db){
	mysql_close(db->con);
}

const char *db_error(T_db *db){
	return mysql_error(db->con);
}
