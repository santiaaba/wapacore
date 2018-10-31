#include "db.h"
#include "config.h"
#include "rest_server.h"
#include "cloud.h"

/********************************
 *      Funciones               *
 ********************************/

void check_clouds(T_list_cloud *clouds){

	T_cloud *cloud;

	printf("Check_cloud\n");
	list_cloud_first(clouds);
	while(!list_cloud_eol(clouds)){
		cloud = list_cloud_get(clouds);
		if(cloud_check(cloud)){
		}
		list_cloud_next(clouds);
	}
}

void check_db(T_db *db){
	// Verifica el estado de la conexion a la base de datos
	printf("Chequeamos base\n");
	if(!db_live(db)){
		printf("DB NO CONNECT!!!!");
		db_connect(db);
	}
	printf("Termina chequeo DB\n");
}

/********************************
 *      Variables GLOBALES      *
 ********************************/
T_config config;
T_db db;
T_rest_server rest_server;
T_list_cloud clouds;
T_logs logs;

void main(){

	//Inicializamos la semilla random
	srand(time(NULL));
	//Levantamos la configuracion 
	if(!config_load("core.conf",&config))
		exit(1);

	if(!logs_init(&logs,config_logs_file(&config),config_logs_level(&config))){
		exit(1);
	}
	logs_write(&logs,L_INFO,"Start Core","");

	printf("Conectamos base de datos\n");
	// Conectamos contra la base de datos
	db_init(&db,&config,&logs);

	if (!db_connect(&db)){
		printf("Imposible conectar a la base de datos\n");
		exit(1);
	}

	// Levantamos de la base de datos las nubes
	list_cloud_init(&clouds);
	if(!db_load_clouds(&db,&clouds)){
		exit(1);
	}

	// Iniciamos el server REST para la API
	rest_server_init(&rest_server,&db,&clouds);

	while(1){
		//check_clouds(&clouds);
		check_db(&db);
		sleep(5);
	}
}
