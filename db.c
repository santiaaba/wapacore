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

int db_load_clouds(T_db *db, T_list_cloud *clouds){
	MYSQL_ROW row;
	T_cloud *new_cloud;

	mysql_query(db->con,"select * from nube");
	MYSQL_RES *result = mysql_store_result(db->con);
	if(result){
		while (row = mysql_fetch_row(result)){
			new_cloud = (T_cloud *)malloc(sizeof(T_cloud));
			cloud_init(new_cloud,atoi(row[0]),atoi(row[2]),row[1],row[3],row[4],row[5],row[6]);
			list_cloud_add(clouds,new_cloud);
		}
		return 1;
	} else {
		return 0;
	}
}

int db_get_cloud_id(T_db *db, int susc_id, T_cloud_type t, int *cloud_id){
	/* Retorna el id de una nube en base al id de una suscripcion
 	 * y al tipo de nube. Recordar que un plan puede tener varias
 	 * nubes pero no mas de una de cada tipo. Si no encuentra la nube
 	 * retorna 0. Caso contrario retorna 1 y en el parametro cloud_id
 	 * el id de la nube */

	char sql[400];
	MYSQL_RES *result;
	MYSQL_ROW row;

	sprintf(sql,"select n.id from nube n inner join plan_nube p on (n.id = p.id_nube) inner join plan pp on (p.id_plan = pp.id) inner join suscription s on (pp.id = s.plan_id) where s.id=%i and n.tipo=%i",susc_id,t);

	mysql_query(db->con,sql);
	result = mysql_store_result(db->con);
	row = mysql_fetch_row(result);
	if(row){
		*cloud_id=atoi(row[0]);
		return 1;
	} else {
		return 0;
	}
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
	char sql[2000];

	strcpy(sql,"select s.id, p.name as plan_name, s.name, s.status, w.hash_dir, wp.quota, wp.sites ");
	strcat(sql,"from suscription s inner join plan p on (s.plan_id = p.id) ");
	strcat(sql,"left join web_suscription w on (s.id = w.id) left join web_plan wp on (s.plan_id = wp.id) ");
	strcat(sql, "where s.id=");
	strcat(sql,susc_id);
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

int db_user_add(T_db *db, T_dictionary *d, char *message){
	
	MYSQL_RES *result;
	MYSQL_ROW row;
	char sql[100];
	
	// Verificamos que el usuario no exista ya */
	sprintf(sql,"select count(name) from user where name='%s'",dictionary_get(d,"name"));
	mysql_query(db->con,sql);
	result = mysql_store_result(db->con);
	row = mysql_fetch_row(result);
	if(atoi(row[0]) >  0){
		strcpy(message,"'Usuario ya existe'");
		return 0;
	}
	sprintf(sql,"insert into user(name,pass,email) values ('%s','%s','%s')",
		dictionary_get(d,"name"),dictionary_get(d,"pass"),dictionary_get(d,"email"));
	if(0 != mysql_query(db->con,sql)){
		strcpy(message,"'Error FATAL'");
		return 0;
	}
	return 1;
}

int db_user_del(T_db *db, T_dictionary *d, char *message){

	char sql[200];
	T_dictionary daux;
	MYSQL_RES *result;
	MYSQL_ROW row;

	printf("DB ELIMINAR usuario\n");
	dictionary_print(d);
	/* Eliminado suscripciones */
	sprintf(sql,"select id from suscription where user_id=%s",dictionary_get(d,"user_id"));
	mysql_query(db->con,sql);
	result = mysql_store_result(db->con);
	dictionary_init(&daux);
	while(row = mysql_fetch_row(result)){
		dictionary_clean(&daux);
		dictionary_add(&daux,"susc_id",row[0]);
		if(!db_susc_del(db,&daux,message)){
			dictionary_clean(&daux);
			return 0;
		}
	}
	dictionary_clean(&daux);
	
	/* Eliminamos usuario */
	sprintf(sql,"delete from user where id=%s",dictionary_get(d,"user_id"));
	printf(sql);
	if(0 != mysql_query(db->con,sql)){
		strcpy(message,"'Error FATAL'");
		return 0;
	}
	return 1;
}

int db_user_mod(T_db *db, T_dictionary *d, char *message){

	char sql[200];
	char sql_aux[200];
	char *aux;
	MYSQL_RES *result;
	MYSQL_ROW row;

	printf("DB modificar usuario\n");
	dictionary_print(d);
	aux = dictionary_get(d,"name");
	if(NULL != aux){
		/* Verificamos que no se repita el nombre del usuario */
		sprintf(sql,"select count(*) from user where name='%s'",aux);
		mysql_query(db->con,sql);
		result = mysql_store_result(db->con);
		row = mysql_fetch_row(result);
		if(atoi(row[0]) >  0){
			strcpy(message,"'Usuario ya existe'");
			return 0;
		}
		strcpy(sql_aux,"set name='" );
		strcat(sql_aux,aux);
		strcat(sql_aux,"'" );
	}

	aux = dictionary_get(d,"email");
	if(NULL != aux){
		if(strlen(sql_aux) == 0)
			strcpy(sql_aux,"set ");
		else
			strcat(sql_aux,",");
		strcat(sql_aux,"email=" );
		strcat(sql_aux,aux);
	}

	aux = dictionary_get(d,"pass");
	if(NULL != aux){
		if(strlen(sql_aux) == 0)
			strcpy(sql_aux,"set ");
		else
			strcat(sql_aux,",");
		strcat(sql_aux,"pass=" );
		strcat(sql_aux,aux);
	}

	sprintf(sql,"update user %s where id=%s",sql_aux,dictionary_get(d,"user_id"));
	printf(sql);
	if(0 != mysql_query(db->con,sql)){
		strcpy(message,"'Error FATAL'");
		return 0;
	}
}

int db_susc_add(T_db *db, T_dictionary *d, char *message){

	MYSQL_RES *result;
	MYSQL_ROW row;
	char sql[100];
	char plan_name[100];
	int max;

	printf("DB ALTA de suscripcion\n");
	// Verificamos la existencia del usuario
	sprintf(sql,"select id,status from user where id=%s",dictionary_get(d,"user_id"));
	mysql_query(db->con,sql);
	result = mysql_store_result(db->con);
	if(!result){
		printf("Usiario con id %s no existe\n",dictionary_get(d,"user_id"));
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
	sprintf(sql,"select id,status,max,name from plan where id=%s",dictionary_get(d,"plan_id"));
	mysql_free_result(result);
	mysql_query(db->con,sql);
	result = mysql_store_result(db->con);
	if(!result){
		printf("Plan con id %s no existe\n",dictionary_get(d,"plan_id"));
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
	sprintf(sql,"select count(*) from suscription where user_id=%s and plan_id=%s",
		dictionary_get(d,"user_id"),dictionary_get(d,"plan_id"));
	mysql_free_result(result);
	mysql_query(db->con,sql);
	result = mysql_store_result(db->con);
	if(atoi(row[0]) > max){
		strcpy(message,"'Usuario posee maximo permitido por plan'");
		return 0;
	}

	// Paso todos los chequeos. Lo damos de alta
	sprintf(sql,"insert into suscription(user_id,plan_id,name,satus) values(%s,%s,'%s',0)",
	dictionary_get(d,"user_id"),dictionary_get(d,"plan_id"),plan_name);
	if(0 != mysql_query(db->con,sql)){
		strcpy(message,"'Error FATAL'");
		return 0;
	}
	return 1;
}

int db_susc_mod(T_db *db, T_dictionary *d, char *message){

	char sql[200];
	char sql_aux[200];
	char *aux;

	printf("DB modificar suscripcion\n");
	dictionary_print(d);
	/* Modificaciones suscripcion padre */
	aux = dictionary_get(d,"name");
	if(NULL != aux){
		strcpy(sql_aux,"set name='" );
		strcat(sql_aux,aux);
		strcat(sql_aux,"'" );
	}

	aux = dictionary_get(d,"status");
	if(NULL != aux){
		if(strlen(sql_aux) == 0)
			strcpy(sql_aux,"set ");
		else
			strcat(sql_aux,",");
		strcat(sql_aux,"status=" );
		strcat(sql_aux,aux);
	}
	sprintf(sql,"update suscription %s where id=%s",sql_aux,dictionary_get(d,"susc_id"));
	printf(sql);
	if(0 != mysql_query(db->con,sql)){
		strcpy(message,"'Error FATAL'");
		return 0;
	}

	/* FALTA QUE ANTE UN CAMBIO DE ESTADO
 	 * DE LA SUSCRIPCION SE VEAN AFECTADOS
 	 * SUS SERVICIOS. POR EJEMPLO QUE LOS
 	 * SITIOS PASEN A ESTAR INHABILITADOS */

	/* Modificacion nube webhosting */
	/* Modificacion nube mysqlDB */
	/* Modificacion nube MSsqlDB */
	return 1;
}

int db_susc_del(T_db *db, T_dictionary *d, char *message){

	char sql[200];

	printf("DB ELIMINAR suscripcion\n");
	dictionary_print(d);

	/* Eliminado nube webhosting */
	/* Eliminado nube mysqlDB */
	/* Eliminado nube MSsqlDB */
	/* Eliminado suscripcion */
	sprintf(sql,"delete from suscription where id=%s",dictionary_get(d,"susc_id"));
	printf(sql);
	if(0 != mysql_query(db->con,sql)){
		strcpy(message,"'Error FATAL'");
		return 0;
	}
}
