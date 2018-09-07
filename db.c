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

void db_user_list(T_db *db, MYSQL_RES **result){

	mysql_query(db->con,"select id,name from user");
	*result = mysql_store_result(db->con);
}

void db_user_show(T_db *db, MYSQL_RES **result,char *id){
	char sql[100];

	sprintf(sql,"select id,name,email from user where id=%s",id);
	mysql_query(db->con,sql);
	*result = mysql_store_result(db->con);
}

int db_user_add(T_db *db, char *name, char *pass, char *email, char *message){
	
	MYSQL_RES *result;
	MYSQL_ROW row;
	char sql[100];
	
	// Verificamos que el usuario no exista ya */
	sprintf(sql,"select count(name) from user where name='%s'",name);
	mysql_query(db->con,sql);
	result = mysql_store_result(db->con);
	row = mysql_fetch_row(result);
	if(atoi(row[0]) >  0){
		strcpy(message,"Usuario ya existe");
		return 0;
	}
	sprintf(sql,"insert into user(name,pass,email) values ('%s','%s','%s')",
		name,pass,email);
	if(0 != mysql_query(db->con,sql)){
		return 0;
		strcpy(message,"Error al crear el usuario en la base de datos");
	}
	return 1;
}
