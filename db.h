/* Posee los metodos y estructuras de datos para manejar el
 * acceso a la base de datos
 */

#include <mysql/mysql.h>
#include "config.h"
#include <stdlib.h>
#include "dictionary.h"

#ifndef DB_H
#define DB_H

typedef struct db {
	MYSQL *con;
} T_db;

void db_init(T_db *db);
int db_connect(T_db *db, T_config *c);
void db_close(T_db *db);
const char *db_error(T_db *db);

/* USERS */
void db_user_list(T_db *db, MYSQL_RES **result);
void db_user_show(T_db *db, MYSQL_RES **result, char *id);
int db_user_add(T_db *db, T_dictionary *d, char *message);
int db_user_mod(T_db *db, T_dictionary *d, char *message);

/* SUSCRIPTION */
void db_susc_list(T_db *db, char *user_id, MYSQL_RES **result);
void db_susc_show(T_db *db, char *susc_id, MYSQL_RES **result);
int db_susc_add(T_db *db, T_dictionary *d, char *message);
int db_susc_mod(T_db *db, T_dictionary *d, char *message);
#endif
