#include "db.h"

/******************************
 * 	T_DB
 ******************************/

/* La mayoria de las funciones son suseptibles a fallar si
 se pierde conexion a la base de datos. Muchas funciones
 retornan si fallaron a causa de conectividad con la base
 de datos en el parametro db_fail
 */

void db_init(T_db *db, T_config *c, T_logs *logs){
	db->con = mysql_init(NULL);
	db->status = DB_ONLINE;
	db->logs = logs;
	strcpy(db->user,c->db_user);
	strcpy(db->pass,c->db_pass);
	strcpy(db->host,c->db_server);
	strcpy(db->dbname,c->db_name);
}

int db_connect(T_db *db){
	if (mysql_real_connect(db->con, db->host,
	     db->user, db->pass, db->dbname, 0, NULL, 0)) {
		db->status = DB_ONLINE;
	} else
		db_close(db);
}

void db_close(T_db *db){
	mysql_close(db->con);
	db->status = DB_OFFLINE;
}

int db_load_clouds(T_db *db, T_list_cloud *clouds){
	/* Levanta las nubes de la base de datos
 	   retorna 0 si no pudo a causa de algun error */
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

int db_get_cloud_id(T_db *db, char *susc_id, T_cloud_type t, int *cloud_id, char *error, int *db_fail){
	/* Retorna el id en el parametro cloud_id de una nube en base al id de una
 	   suscripcion pasada en el parametro susc_id. db_fail retorna si hubo falla
 	   Recordar que un plan puede tener varias nubes pero no mas de una de cada
	   tipo. Si no encuentra la nube retorna 0. Caso contrario retorna 1 */

	char sql[400];
	MYSQL_RES *result;
	MYSQL_ROW row;

	*cloud_id=0;
	sprintf(sql,"select n.id from nube n inner join plan_nube p on (n.id = p.id_nube) inner join plan pp on (p.id_plan = pp.id) inner join suscription s on (pp.id = s.plan_id) where s.id=%s and n.tipo=%i",susc_id,t);

	printf("db_get_cloud_id: %s\n",sql);
	if(mysql_query(db->con,sql)){
		*db_fail = 1;
		return 0;
	}
	printf("PASO A\n");
	*db_fail = 0;
	result = mysql_store_result(db->con);
	if((mysql_num_rows(result) == 0)){
		sprintf(error,"{\"code\":\"350\",\"info\":\"Nube no existe\"}");
		return 0;
	}
	printf("PASO B\n");
	row = mysql_fetch_row(result);
	*cloud_id=atoi(row[0]);
	printf("PASO C\n");
	return 1;
}

int db_user_exist(T_db *db, T_dictionary *d, char *error, int *db_fail){
	/* Retorna 0 si el usuario no existe y retorna mensaje en *message.
	   1 si existe. Si falla el acceso a la base lo indica en db_fail */ 

	char sql[400];
	MYSQL_RES *result;

	printf("DB_USER_EXIST\n");
	sprintf(sql,"select id from user where id=%s",dictionary_get(d,"user_id"));
	printf("DB_USER_EXIST: %s\n",sql);
	mysql_query(db->con,sql);
	if(mysql_errno(db->con)){
		*db_fail = 1;
		return 0;
	}
	result = mysql_store_result(db->con);
	*db_fail = 0;
	if(mysql_num_rows(result) == 0){
		strcpy(error,"{\"code\":\"312\",\"info\":\"usuario existente\"}");
		return 0;
	} else 
		return 1;
}

int db_susc_exist(T_db *db, T_dictionary *d, char *error, int *db_fail){
	/* Retorna 0 si la suscripcion no existeretorna mensaje en *message.
	   1 si existe. Si falla el acceso a la base lo indica en db_fail */ 
		
	char sql[100];
	MYSQL_RES *result;
        MYSQL_ROW row;

	printf("DB_SUSC_EXIST\n");
	/* Verificamos primero existencia del usuario */
	if(db_user_exist(db,d,error,db_fail)){
		sprintf(sql,"select count(id) from suscription where id=%s and user_id=%s",
			dictionary_get(d,"susc_id"),dictionary_get(d,"user_id"));
		printf("DB_SUSC_EXIST: %s\n",sql);
		mysql_query(db->con,sql);
		if(mysql_errno(db->con)){
			*db_fail = 1;
			return 0;
		}
		*db_fail = 0;
		result = mysql_store_result(db->con);
		if(mysql_num_rows(result) == 0){
			strcpy(error,"{\"code\":\"310\",\"info\":\"Suscripcion no existente\"}");
       	 	        return 0;
       	 	} else 
			return 1;
	}
}

int db_susc_prepare(T_db *db, T_dictionary *d, int action){
	/* Prepara una suscripcion para ser modificada o eliminada.
	   Retorna 0 si falla la base de datos */
	char sql[100];

	sprintf(sql,"update suscription set last_action = %i, status=2 where id = %s",
                dictionary_get(d,"susc_id"));
	if(mysql_query(db->con,sql))
		return 0;
	return 1;
}

void db_user_list(T_db *db, MYSQL_RES **result, int *db_fail){

	mysql_query(db->con,"select id,name from user");
	if(mysql_errno(db->con)){
		*db_fail = 1;
	}
	*result = mysql_store_result(db->con);
	*db_fail = 0;
}

int db_user_show(T_db *db, T_dictionary *d, MYSQL_RES **result, char *error, int *db_fail){
	/* Retorna los datos de un usuario basado en el id.
 	   Si el usuario no existe retorna 0. Sino retorna 1. */
	char sql[100];

	sprintf(sql,"select id,name,email,status from user where id=%s", dictionary_get(d,"user_id"));
	mysql_query(db->con,sql);
	if(mysql_errno(db->con)){
		*db_fail = 1;
		return 0;
	}
	*db_fail = 0;
	*result = mysql_store_result(db->con);
	if(mysql_num_rows(*result))
		return 1;
	else {
		sprintf(error,"{\"code\":\"320\",\"info\":\"Usuario inexistente\"}");
		return 0;
	}
}

int db_susc_show(T_db *db, T_dictionary *d, char **data, char *error, int *db_fail){
	/* Retorna datos de la suscripcion en base al susc_id. Si no existe
 	   retorna 0. Si existe retorna 1. */
	char sql[200];
	char aux[400];
	MYSQL_ROW row;
	MYSQL_RES *result;

	strcpy(sql,"select name,status,plan_id from suscription where id=");
	strcat(sql,dictionary_get(d,"susc_id"));
	printf("%s\n",sql);
	mysql_query(db->con,sql);
	if(mysql_errno(db->con)){
		*db_fail = 1;
		return 0;
	}
	*db_fail = 0;
	result = mysql_store_result(db->con);
	if(mysql_num_rows(result)){
		row = mysql_fetch_row(result);
		sprintf(aux,"{\"code\":\"200\",\"info\":{\"name\":\"%s\",\"status\":\"%s\",\"plan_id\":\"%s\",\"clouds\":{",
		row[0],row[1],row[2]);
		*data = (char *)realloc(*data,strlen(aux)+1);
		strcpy(*data,aux);
		return 1;
	} else {
		strcpy(error,"{\"code\":\"310\",\"info\":\"Suscipcion inexistente\"}");
		return 0;
	}
}

int db_susc_list(T_db *db, T_dictionary *d, MYSQL_RES **result, char *error, int *db_fail){
	/* Retorna el listado de suscripciones de un usuario dado su id.
	   Si el usuario no existe retorna el error en *error.
	   Si no hay errores retorna el resultado de la consulta en **result
	   Si falla la conexion a la base lo indica en db_fail
 	*/
	char sql[150];

	if(db_user_exist(db,d,error,db_fail)){
		sprintf(sql,"select s.id, s.name, p.name as plan_name from suscription s inner join plan p on (s.plan_id = p.id) where user_id=%s", dictionary_get(d,"user_id"));
		mysql_query(db->con,sql);
		if(mysql_errno(db->con)){
			*db_fail = 1;
			return 0;
		}
		*db_fail = 0;
		*result = mysql_store_result(db->con);
		return 1;
	} else
		return 0;
}

int db_user_add(T_db *db, T_dictionary *d, char *error, int *db_fail){
	/* Agrega un usuario. Retorna 1 si pudo agregarlo y 0 si no.
 	   Si no pudo agregarlo en *mensaje retorna el motivo en formato
	   json. */
	
	MYSQL_RES *result;
	char sql[100];
	
	sprintf(sql,"insert into user(name,pass,email) values ('%s','%s','%s')",
		dictionary_get(d,"name"),dictionary_get(d,"pass"),dictionary_get(d,"email"));
	printf("DB_user_add: %s\n",sql);
	mysql_query(db->con,sql);
	if(mysql_errno(db->con)){
		printf("Hay error %lu\n",mysql_errno(db->con));
		if (mysql_errno(db->con) == 1062){	//ER_DUP_ENTRY
			*db_fail = 0;
			strcpy(error,"{\"code\":\"312\",\"info\":\"usuario existente\"}");
		} else {
			*db_fail = 1;
		}
		return 0;
	}
	*db_fail = 0;
	return 1;
}

int db_user_del(T_db *db, T_dictionary *d, char *error, int *db_fail){
	/* Elimina un usuario y todas sus suscripciones.
 	   Retorna 0 si no pudo y en *message el motivo
 	   Retorna 1 si pudo eliminarlo. Si hubo errores contra
	   la base de datos retorna db_fail = 1. 0 en caso contrario */

	char sql[200];
	T_dictionary daux;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int db_fail_aux;

	dictionary_print(d);
	/* Eliminado suscripciones */
	sprintf(sql,"select id from suscription where user_id=%s",dictionary_get(d,"user_id"));
	mysql_query(db->con,sql);
	result = mysql_store_result(db->con);
	if(!result){
		*db_fail = 1;
		return 0;
	}
	dictionary_init(&daux);
	while(row = mysql_fetch_row(result)){
		dictionary_add(&daux,"susc_id",row[0]);
		if(!db_susc_del(db,&daux)){
			if(db_fail_aux){
				/* Fallo la base de datos */
				*db_fail=1;
			}
			dictionary_clean(&daux);
			return 0;
		}
	}
	dictionary_clean(&daux);
	
	/* Eliminamos usuario */
	sprintf(sql,"delete from user where id=%s",dictionary_get(d,"user_id"));
	if(0 != mysql_query(db->con,sql)){
		*db_fail=1;
		return 0;
	}
	strcpy(error,"{\"code\":\"222\",\"info\":\"Usuario eliminado\"}");
	*db_fail=0;
	return 1;
}

int db_user_mod(T_db *db, T_dictionary *d, char *error, int *db_fail){

	char sql[200];
	char sql_aux[200];
	char *aux;
	MYSQL_RES *result;
	MYSQL_ROW row;

	dictionary_print(d);
	aux = dictionary_get(d,"name");
	if(NULL != aux){
		/* Verificamos que no se repita el nombre del usuario */
		sprintf(sql,"select count(*) from user where name='%s'",aux);
		mysql_query(db->con,sql);
		result = mysql_store_result(db->con);
		if(!result){
			*db_fail=1;
			return 0;
		}
		*db_fail=0;
		row = mysql_fetch_row(result);
		if(atoi(row[0]) >  0){
			strcpy(error,"{\"code\":\"321\",\"info\":\"Usuario ya existe\"}");
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
	printf("DB_USER_MOD: %s\n",sql);
	if(0 != mysql_query(db->con,sql)){
		*db_fail=1;
		return 0;
	}
	return 1;
}

int db_user_stop(T_db *db, T_dictionary *d, char *error, int *db_fail){
	/* Inhabilita en la base de datos un usuario.
 	   El usuario puede no exisitr */
	char sql[100];

	sprintf(sql,"update user set status=0 where id=%s",dictionary_get(d,"user_id"));
	if(mysql_query(db->con,sql) != 0){
		/* Algo fallo */
		*db_fail=1;
		return 0;
	} else {
		*db_fail=0;
		if(mysql_affected_rows(db->con) == 1)
			return 1;
		else 
			sprintf("error","Usuario no existe");
			return 0;
	}
}

int db_user_start(T_db *db, T_dictionary *d, char *error, int *db_fail){
	/* habilita en la base de datos un usuario */
	char sql[100];

	sprintf(sql,"update user set status=1 where id=%s",dictionary_get(d,"user_id"));
	if(mysql_query(db->con,sql) != 0){
		/* Algo fallo */
		*db_fail=1;
		return 0;
	} else {
		*db_fail=0;
		if(mysql_affected_rows(db->con) == 1)
			return 1;
		else 
			sprintf("error","Usuario no existe");
			return 0;
	}
}

int db_susc_broken(T_db *db, T_dictionary *d, char *error, int *db_fail){
	/* Coloca una suscripcion en Broken */

	char sql[100];
	sprintf(sql,"update suscription set status=3 where id=%s",dictionary_get(d,"susc_id"));
	printf(sql);
	if(mysql_query(db->con,sql)){
		*db_fail=1;
		return 0;
	}
	*db_fail=0;
	return 1;
}

int db_susc_add(T_db *db, T_dictionary *d, char *error, int *db_fail){
	/* Agrega una suscripcion a la base de datos. Si pudo
 	 * agregarla retorna 0 y suma al diccionario su valor bajo
 	 * la key "susc_id". Si no pudo retorna 0 */
	/* Si la base falla retorna db_fail = 1. Caso contrario 0. */
	/* No realiza verificaciones. Ver funcion db_susc_add_ok */

	MYSQL_RES *result;
	MYSQL_ROW row;
	char susc_id_char[50];
	char sql[100];
	char hash_dir[6];
	char plan_name[100];
	int max;
	my_ulonglong susc_id;

	*db_fail=0;
	printf("DB_SUSC_ADD\n");
	// Verificamos la existencia del usuario
	sprintf(sql,"select id,status from user where id=%s",dictionary_get(d,"user_id"));
	if(mysql_query(db->con,sql)){
		*db_fail=1;
		return 0;
	}
	result = mysql_store_result(db->con);
	if(mysql_num_rows(result) == 0){
		strcpy(error,"{\"code\":\"320\",\"info\":\"Usuario inexistente\"}");
		return 0;
	}
	printf("paso 1\n");
	// Verificamos que el usuario este activo
	row = mysql_fetch_row(result);
	if(atoi(row[1]) == 0){
		strcpy(error,"{\"code\":\"322\",\"info\":\"Usuario inactivo\"}");
		return 0;
	}
	
	printf("paso 2\n");
	// Verificamos la existencia del plan
	sprintf(sql,"select name,max,status from plan where id=%s",dictionary_get(d,"plan_id"));
	if(mysql_query(db->con,sql)){
		*db_fail=1;
		return 0;
	}
	result = mysql_store_result(db->con);
	if(mysql_num_rows(result) == 0){
		strcpy(error,"{\"code\":\"330\",\"info\":\"Plan no existe\"}");
                return 0;
	}
	// Verificamos que el plan este instanciable
	printf("paso 3\n");
	printf("paso 3\n");
	row = mysql_fetch_row(result);
	printf("paso 3 - %s\n",row[2]);
	if(atoi(row[2]) == 0){
		strcpy(error,"{\"code\":\"331\",\"info\":\"Plan no instanciable\"}");
                return 0;
	} else {
		max = atoi(row[1]);
		strcpy(plan_name,row[0]);
	}
	
	printf("paso 4\n");
	// Verificamos que el usuario no supere la cantidad de instancias del mismo plan
	sprintf(sql,"select count(*) from suscription where user_id=%s and plan_id=%s",
		dictionary_get(d,"user_id"),dictionary_get(d,"plan_id"));
	if(mysql_query(db->con,sql)){
		*db_fail=1;
		return 0;
	}
	printf("paso 5\n");
	result = mysql_store_result(db->con);
	row = mysql_fetch_row(result);
	if(atoi(row[0]) > max){
		strcpy(error,"{\"code\":\"332\",\"info\":\"Maximo de suscripciones para el plan alcanzado\"}");
		return 0;
	}

	printf("paso 6\n");
	// Paso todos los chequeos. Lo damos de alta. Last_action = 1
	sprintf(sql,"insert into suscription(user_id,plan_id,name,last_action) values(%s,%s,'%s',1)",
	dictionary_get(d,"user_id"),dictionary_get(d,"plan_id"),plan_name);
	if(mysql_query(db->con,sql)){
		*db_fail=1;
		return 0;
	}
	susc_id = mysql_insert_id(db->con);
	printf("paso 7\n");
	sprintf(susc_id_char,"%lu",susc_id);
	printf("suscription_id nuevo %s\n",susc_id_char);
	dictionary_add(d,"susc_id",susc_id_char);
	strcpy(error,"{\"code\":\"211\",\"info\":\"Suscripcion agregada\"}");
	*db_fail=0;
	return 1;
}

int db_susc_add_active(T_db *db, T_dictionary *d){
	/* Luego de la creacion de la suscripcion y de sus nubes... hay que activarla */

	char sql[200];
	sprintf(sql,"update suscription set status=1 where id=%s",dictionary_get(d,"susc_id"));
	if(mysql_query(db->con,sql)){
		return 0;
	}
	return 1;
}

int db_susc_mod(T_db *db, T_dictionary *d, char *error, int *db_fail){
	/* Modifica una suscripcion. */
	/* No verifica que exista la suscripcion. Es responsabilidad
 	 * de la funcion llamadora */

	char sql[200];
	char sql_aux[200];
	char *aux;

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
	if(0 != mysql_query(db->con,sql)){
		*db_fail=1;
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

int db_susc_del(T_db *db, T_dictionary *d){
	/* Elimina una suscripcion de la base de datos. Retorna 1 si falla la base
 	 * de datos. No verifica si la suscripcion existe o es del usuario correcto,
 	 * es responsabilidad de la funcion llamadora */

	char sql[200];

	/* Eliminado suscripcion */
	sprintf(sql,"delete from suscription where id=%s",dictionary_get(d,"susc_id"));
	printf("sql: %s\n",sql);
	if(mysql_query(db->con,sql))
		return 0;
	return 1;
}

int db_susc_stop(T_db *db, T_dictionary *d, char *error, int *db_fail){
	/* Detenemos las suscripciones en la base de datos */
	/* NO verifica la existencia de la suscripcion o a que
 	 * usuario corresponde. Es responsabilidad de la funcion llamadora */
	char sql[200];

	sprintf(sql,"update suscription set status=0 where id=%s",dictionary_get(d,"susc_id"));
	if(0 != mysql_query(db->con,sql)){
		*db_fail=1;
		return 0;
	}
	*db_fail=0;
	return 1;

}

int db_susc_start(T_db *db, T_dictionary *d, char *error, int *db_fail){
	/* Iniciamos las suscripciones en la base de datos */
	/* NO verifica la existencia de la suscripcion 
 	 * usuario corresponde. Es responsabilidad de la funcion llamadora */
	char sql[200];

	sprintf(sql,"update suscription set status=1 where id=%s",dictionary_get(d,"susc_id"));
	if(0 != mysql_query(db->con,sql)){
		*db_fail=1;
		return 0;
	}
	*db_fail=0;
	return 1;
}

int db_accept_add_site(T_db *db, T_dictionary *d, char *error, int *db_fail){

	/* Verificamos que suscripcion sea del usuario */
	/* Verificamos cantidad de sitios permitidos en suscripcion */

	*db_fail=0;
	return 1;
}
