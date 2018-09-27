/* Posee los metodos y estructuras de datos para manejar el
 * acceso a la base de datos
 */

#include <mysql/mysql.h>
#include "config.h"
#include <stdlib.h>
#include "dictionary.h"
#include "cloud.h"

#ifndef DB_H
#define DB_H

typedef enum { DB_ONLINE, DB_OFFLINE} T_DB_status;

typedef struct db {
	MYSQL *con;
	char user[100];
	char pass[100];
	char host[100];
	char dbname[100];
	T_DB_status status;
} T_db;

void db_init(T_db *db, T_config *c);
int db_connect(T_db *db);
void db_close(T_db *db);
const char *db_error(T_db *db);

int db_load_clouds(T_db *db, T_list_cloud *clouds);
int db_get_cloud_id(T_db *db, int susc_id, T_cloud_type t, int *cloud_id);

/* USERS */
int db_user_list(T_db *db, MYSQL_RES **result);
int db_user_show(T_db *db, MYSQL_RES **result, char *id);
int db_user_add(T_db *db, T_dictionary *d, char *message);
int db_user_mod(T_db *db, T_dictionary *d, char *message);

/* SUSCRIPTION */
void db_susc_list(T_db *db, char *user_id, MYSQL_RES **result);
void db_susc_show(T_db *db, char *susc_id, MYSQL_RES **result);
int db_susc_add(T_db *db, T_dictionary *d, char *message);
int db_susc_mod(T_db *db, T_dictionary *d, char *message);
int db_susc_exist(T_db *db, T_dictionary *d, char *message);
int db_susc_del(T_db *db, T_dictionary *d, char *message);

/* SITIOS */
int db_accept_add_site(T_db *db, char *susc_id);

#endif
