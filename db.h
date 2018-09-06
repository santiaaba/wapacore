/* Posee los metodos y estructuras de datos para manejar el
 * acceso a la base de datos
 */

#include <mysql/mysql.h>
#include "config.h"
#include <stdlib.h>

#ifndef DB_H
#define DB_H

typedef struct db {
	MYSQL *con;
} T_db;

void db_init(T_db *db);
int db_connect(T_db *db, T_config *c);
void db_close(T_db *db);
const char *db_error(T_db *db);

#endif
