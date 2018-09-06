#include "db.h"
#include "config.h"
#include "rest_server.h"

/********************************
 *      Variables GLOBALES      *
 ********************************/
T_config config;
T_db db;
T_rest_server rest_server;

void main(){

	//Inicializamos la semilla random
	srand(time(NULL));

	//Levantamos la configuracion 
	if(!config_load("core.conf",&config))
		exit(1);

	printf("db_server : %s\n",config_db_server(&config));
	printf("db_user : %s\n",config_db_user(&config));
	printf("db_pass : %s\n",config_db_pass(&config));
	printf("db_name : %s\n",config_db_name(&config));
	

	// Conectamos contra la base de datos
	db_init(&db);
	if (!db_connect(&db,&config)){
		printf("Error coneccion a la base de datos: %s\n",db_error(&db));
		exit(1);
	}
	// Iniciamos el server REST para la API
	rest_server_init(&rest_server,&db);
	printf("El rest server posee puntero %p\n",rest_server);
	while(1){
	}
}
