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

void db_susc_show(T_db *db, char *susc_id, MYSQL_RES **result){
	char sql[200];

	sprintf(sql,"select s.id, p.name as plan_name, s.name, s.status from suscription s inner join plan p on (s.plan_id = p.id) where s.id=%s",susc_id);
	printf("SQL: %s\n",sql);
	mysql_query(db->con,sql);
	*result = mysql_store_result(db->con);
}

void db_susc_list(T_db *db, char *user_id, MYSQL_RES **result){
	char sql[150];

	sprintf(sql,"select s.id, s.name, p.name as plan_name from suscription s inner join plan p on (s.plan_id = p.id) where user_id=%s",user_id);
	printf("SQL: %s\n",sql);
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
		strcpy(message,"'Usuario ya existe'");
		return 0;
	}
	sprintf(sql,"insert into user(name,pass,email) values ('%s','%s','%s')",
		name,pass,email);
	if(0 != mysql_query(db->con,sql)){
		return 0;
		strcpy(message,"'Error FATAL'");
	}
	return 1;
}

int db_suscrip_add(T_db *db, char *user_id, char *plan_id, char *message){

	MYSQL_RES *result;
	MYSQL_ROW row;
	char sql[100];
	char plan_name[100];
	int max;

	printf("DB ALTA de suscripcion\n");
	// Verificamos la existencia del usuario
	sprintf(sql,"select id,status from user where id=%s",user_id);
	mysql_query(db->con,sql);
	result = mysql_store_result(db->con);
	if(!result){
		printf("Usiario con id %s no existe\n",user_id);
		strcpy(message,"'Usuario no existe'");
		return 0;
	}
	// Verificamos que el usuario este activo
	row = mysql_fetch_row(result);
	if(row[1] > 0){
		strcpy(message,"'Usuario no esta activo'");
		return 0;
	}
	
	// Verificamos la existencia del plan
	sprintf(sql,"select id,status,max,name from plan where id=%s",plan_id);
	mysql_free_result(result);
	mysql_query(db->con,sql);
	result = mysql_store_result(db->con);
	if(!result){
		printf("Plan con id %s no existe\n",plan_id);
		strcpy(message,"'Plan no existe'");
		return 0;
	}
	// Verificamos que el plan este instanciable
	row = mysql_fetch_row(result);
	if(row[1] > 0){
		strcpy(message,"'Plan no instanciable'");
		return 0;
	} else {
		strcpy(plan_name,row[3]);
		max = atoi(row[2]);
	}
	
	// Verificamos que el usuario no supere la cantidad de instancias del mismo plan
	sprintf(sql,"select count(*) from suscription where user_id=%s and plan_id=%s",user_id,plan_id);
	mysql_free_result(result);
	mysql_query(db->con,sql);
	result = mysql_store_result(db->con);
	if(atoi(row[0]) > max){
		strcpy(message,"'Usuario posee maximo permitido por plan'");
		return 0;
	}

	// Paso todos los chequeos. Lo damos de alta
	sprintf(sql,"insert into suscription(user_id,plan_id,name,satus) values(%s,%s,'%s',0)",
	user_id,plan_id,plan_name);
	if(0 != mysql_query(db->con,sql)){
		return 0;
		strcpy(message,"'Error FATAL'");
	}
	return 1;
}
