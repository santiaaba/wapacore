#include "task.h"

void random_task_id(T_taskid value){
	/*Genera un string random para task_id */
	char *string = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	int i,j;
	
	for(j=0;j<TASKID_SIZE;j++){
		i = rand() % 62;
		value[j] = string[i];
	}
}

void random_token(T_tasktoken value){
	/*Genera un string random para token_id */
	char *string = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	int i,j;

	for(j=0;j<TOKEN_SIZE;j++){
		i = rand() % 62;
		value[j] = string[i];
	}
}

/*****************************
	     TASK 
******************************/
void task_init(T_task *t, T_tasktoken *token, T_dictionary *data, T_logs *logs){
	printf("dictionary:");
	dictionary_print(data);
	random_task_id(t->id);
	t->token = token;
	t->type = T_NONE;
	t->logs = logs;
	t->data = data;
	t->step=0;
	t->status = T_TODO;
	t->result = (char *)malloc(TASKRESULT_SIZE);
	t->result_code = 0;
	t->cloud = NULL;
	strcpy(t->result,"");
}

void task_set_type(T_task *t, T_task_type type){
	t->type = type;
}

char *task_get_id(T_task *t){
	return t->id;
}

void task_destroy(T_task **t){
	printf("Entramos a task_destroy\n");
	if((*t)->data != NULL){
		printf("Destruimos el diccionario\n");
		dictionary_destroy(&((*t)->data));
		printf("Fin Destruimos el diccionario\n");
	}
	free((*t)->result);
	free(*t);
}

T_task_status task_get_status(T_task *t){
	return t->status;
}

T_tasktoken *task_get_token(T_task *t){
	return t->token;
}

T_dictionary *task_get_data(T_task *t){
	return t->data;
}

void task_print_status(T_task *t, char *s){
	switch(t->status){
	case T_WAITING: strcpy(s,"waiting"); break;
	case T_TODO: strcpy(s,"todo"); break;
	case T_DONE: strcpy(s,"done"); break;
	}
}

void task_done(T_task *t, char *message){
	/* Completa los datos para indicar que un task ha terminado */
	printf("Entro\n");
	t->status = T_DONE;
	printf("Largo: %i\n",strlen(message)+1);
	t->result=(char *)realloc(t->result,strlen(message)+1);
	strcpy(t->result,message);
	printf("PASO\n");
}

char *tob_get_id(T_task *t){
	return t->id;
}

int task_assign_cloud(T_task *t, T_list_cloud *cl){
	/* Cuando son acciones directas sobre una nube, el task
 	 * deberia tener un dato en el diccionario llamado cloud_id.
 	 * Con el cloud_id debe buscar la nube en la lista y asignarsela */
	t->cloud = list_cloud_find_id(cl,atoi(dictionary_get(t->data,"cloud_id")));
	return (t->cloud != NULL);
}

/*************	TASK SUSCRIPT  ***********************/

int task_susc_add(T_task *t, T_db *db, T_list_cloud *cl){
	/* Agrega una suscripcion. Se conecta a las
 	 * nubes que correspondan para dar de alta
 	 * los elementos de la suscripcion */

	int ok;
	char result[100];
	char *susc_id;
	int cloud_id;
	char send_message[100];
	T_cloud *c = NULL;
	char error[200];
	int db_fail;

	/* Damos de alta la suscripcion en la base de datos del Core
 	   si corresponde */
	printf("Task alta suscripcion\n");
	if(t->status == T_TODO){
		if(t->step == 0){
			/* Creamos la suscripcion en la base de datos. Que da en estado "pendiente" */
			if(!db_susc_add(db,t->data,error,&db_fail)){
				if(db_fail)
					task_done(t,ERROR_FATAL);
				else
					task_done(t,error);
			}
		} else if(t->step > 0 && t->step < 4){
			/* Accionamos sobre las nubes */
			susc_id = dictionary_get(t->data,"susc_id");
			switch (t->step){
				case 1:	ok = db_get_cloud_id(db,susc_id,C_WEB,&cloud_id,error,&db_fail);
					break;
				case 2: ok = db_get_cloud_id(db,susc_id,C_MYSQLDB,&cloud_id,error,&db_fail);
					break;
				case 3: ok = db_get_cloud_id(db,susc_id,C_MSSQLDB,&cloud_id,error,&db_fail);
					break;
			}
			if(ok){
				/* La suscripcion utliza esta nube */
				c = list_cloud_find_id(cl,cloud_id);
				t->cloud = c;
				sprintf(send_message,"0susc_id|%s", susc_id);
				if(!task_cloud_send(t,send_message)){
					printf("Nube no responde %i\n",t->step);
					task_done(t,"{\"code\":\"300\",\"info\":\"Nube inaccesible. Suscripcion pendiente\"}");
				}
			} else 
				if(db_fail)
					task_done(t,ERROR_FATAL);
		}
		if(t->step == 4){
			/* Hemos podido crear la suscripcion. Hemos podido crear la suscripcion en
 			 * las distintas nubes.Activamos definitivamente la suscripcion. */
			if(!db_susc_add_active(db,t->data))
				task_done(t,"{\"code\":\"300\",\"info\":\"Error Base de datos. Suscripcion pendiente\"}");
			else
				task_done(t,"{\"code\":\"211\",\"info\":\"Suscripcion agregada\"}");
		}
		t->step ++;	// dejo preparado para ejecutar el paso siguiente
	} else if(t->status == T_WAITING){
		task_cloud_get(t);
		printf("Terminamos recibir mensaje de nube\n");
		if(t->status == T_DONE){
			if(t->result_code == 200){
				/* La llamada a la nube retorno que genero el alta
 				 * pasamos al paso siguiente. */
				if(t->step < 3){
					t->status = T_TODO;
				} else {
					task_done(t,"{\"code\":\"211\",\"info\":\"Suscripcion agregada\"}");
				}
			} else {
				/* Error retornado por la nube */
				if(!db_susc_broken(db,t->data,error,&db_fail)){
					if(db_fail)
						task_done(t,ERROR_FATAL);
					else
						task_done(t,error);
				}
			}
		}
	}
}

int task_susc_del(T_task *t, T_db *db, T_list_cloud *cl){
	int ok;
	char result[100];
	char error[200];
	char *susc_id;
	int cloud_id;
	char send_message[100];
	T_cloud *c = NULL;
	int db_fail;

	/* Verificamos que ese suscripción exista para el
 	 * usuario indicado */

	susc_id = dictionary_get(t->data,"susc_id");
	if(t->status == T_TODO){
		printf("PASOOOOO: %i\n",t->step);
		if(t->step == 0){
			/* Realizamos chequeos previos */
			if(!db_susc_exist(db,t->data,error,&db_fail)){
				if(db_fail)
					task_done(t,ERROR_FATAL);
				else
					task_done(t,error);
			} else
				if(!db_susc_prepare(db,t->data,2))
					task_done(t,ERROR_FATAL);
		} else if(t->step > 0 && t->step < 4){
			susc_id = dictionary_get(t->data,"susc_id");
			switch (t->step){
				case 1: ok = db_get_cloud_id(db,susc_id,C_WEB,&cloud_id,error,&db_fail);
					break;
				case 2: ok = db_get_cloud_id(db,susc_id,C_MYSQLDB,&cloud_id,error,&db_fail);
					break;
				case 3: ok = db_get_cloud_id(db,susc_id,C_MSSQLDB,&cloud_id,error,&db_fail);
					break;
			}
			if(ok){
				c = list_cloud_find_id(cl,cloud_id);
				t->cloud = c;
				sprintf(send_message,"1susc_id|%s", susc_id);
				task_cloud_send(t,send_message);
			} else
				if(db_fail)
					task_done(t,ERROR_FATAL);
		} else if(t->step == 4){
			/* Accionamos sobre el CORE */
			if(!db_susc_del(db,t->data))
				task_done(t,ERROR_FATAL);
			else
				task_done(t,"{\"code\":\"212\",\"info\":\"Suscripcion eliminada\"}");
		}
		t->step ++;	// dejo preparado para ejecutar el paso siguiente
	} else if(t->status == T_WAITING){
		task_cloud_get(t);	//Esto me coloca el estado en T_DONE
		if(t->status == T_DONE){
			if(t->result_code == 200){
				t->status = T_TODO;
			}
		}
	}
}

int task_susc_mod(T_task *t, T_db *db){
	/* Modifica los valores de una suscripcion */
	char result[100];
	char error[200];
	int db_fail;

	if(!db_susc_exist(db,t->data,error,&db_fail)){
		if(db_fail)
			task_done(t,ERROR_FATAL);
		else
			task_done(t,"{\"code\":\"310\",\"info\":\"Suscripcion inexistente\"}");
	} else {
		if(db_susc_mod(db,t->data,result,&db_fail)){
			task_done(t,"{\"code\":\"213\",\"info\":\"Suscripcion modificada\"}");
		} else {
			if(db_fail)
				task_done(t,ERROR_FATAL);
			else {
				task_done(t,result);
			}
		}
	}
}

void task_susc_show(T_task *t, T_db *db, T_list_cloud *cl){
	char *aux=NULL;
	char error[200];
	int db_fail;
        int cloud_id;
        T_cloud *c = NULL;
        char send_message[100];
	char *susc_id;
	int ok;

	if(t->status == T_TODO){
		if(t->step == 0){
			/* Realizamos chequeos previos */
			if(!db_susc_exist(db,t->data,error,&db_fail)){
				if(db_fail)
					task_done(t,ERROR_FATAL);
				else
					task_done(t,error);
			}
		} else if(t->step > 0 && t->step < 4){
			susc_id = dictionary_get(t->data,"susc_id");
			switch (t->step){
				case 1: ok = db_get_cloud_id(db,susc_id,C_WEB,&cloud_id,error,&db_fail);
					break;
				case 2: ok = db_get_cloud_id(db,susc_id,C_MYSQLDB,&cloud_id,error,&db_fail);
					break;
				case 3: ok = db_get_cloud_id(db,susc_id,C_MSSQLDB,&cloud_id,error,&db_fail);
					break;
			}
			if(ok){
				c = list_cloud_find_id(cl,cloud_id);
				t->cloud = c;
				sprintf(send_message,"4susc_id|%s", susc_id);
				task_cloud_send(t,send_message);
			} else {
				if(db_fail)
					task_done(t,ERROR_FATAL);
			}
		} else if(t->step == 4){
			/* Accionamos sobre el CORE */
			if(!db_susc_show(db,t->data,&aux,error,&db_fail))
				task_done(t,error);
			else {
				printf("Obtuvimos datos de la suscripcion: %s\n",aux);
				aux = (char *)realloc(aux,strlen(aux) + strlen(t->result) + 3);
				strcat(aux,t->result);
				t->result = (char *)realloc(t->result,strlen(aux));
				strcpy(t->result,aux);
				strcat(t->result,"}}");
				t->status = T_DONE;
			}
			free(aux);
		}
		t->step ++;	// dejo preparado para ejecutar el paso siguiente
	} else if(t->status == T_WAITING){
		aux = malloc(strlen(t->result));
		strcpy(aux,t->result);
		task_cloud_get(t);	//Esto me coloca el estado en T_DONE
		printf("STATUS_CODE: %i\n",t->result_code);
		if(t->result_code == 200){
			//Agregamos los datos retornados por cada nube 
			aux = (char *)realloc(aux,strlen(aux) + strlen(t->result));
			strcat(aux,t->result);
			printf("Obtuvimos datos de la la nube: %s\n",aux);
			t->result = (char *)realloc(t->result,strlen(aux));
			strcpy(t->result,aux);
			t->status = T_TODO;
		} else {
			task_done(t,ERROR_FATAL);
			printf("NO obtuvimos datos de la la nube\n");
		}
		free(aux);
	}
}

void task_susc_list(T_task *t, T_db *db){
	MYSQL_RES *result;
	char error[200];
	char *aux=NULL;
	int db_fail;

	printf("TASK_SUSC_LIST\n");
	if(!db_susc_list(db,t->data,&result,error,&db_fail)){
		if(db_fail)
			task_done(t,ERROR_FATAL);
		else
			task_done(t,error);
	} else {
		json_susc_list(&aux,result);
		task_done(t,aux);
	}
	free(aux);
}

void task_susc_stop(T_task *t, T_db *db, T_list_cloud *cl){
	/* Le indicamos a las distintas nubes que forman parte
 	   de la suscripcion que detengan todos los elementos
	   de la suscripcion */
	int ok;
	char *susc_id;
	char error[200];
	int db_fail;
	int cloud_id;
	T_cloud *c = NULL;
	char send_message[100];

	if(t->status == T_TODO){
		if(t->step == 0){
			/* Realizamos chequeos previos */
			if(!db_susc_exist(db,t->data,error,&db_fail)){
				if(db_fail)
					task_done(t,ERROR_FATAL);
				else
					task_done(t,error);
			} else
				if(!db_susc_prepare(db,t->data,3))
					task_done(t,ERROR_FATAL);
			
		} else 	if(t->step > 0 && t->step < 4){
			/* Accionamos sobre las nubes */
			susc_id = dictionary_get(t->data,"susc_id");
			switch (t->step){
				case 1: ok = db_get_cloud_id(db,susc_id,C_WEB,&cloud_id,error,&db_fail);
					break;
				case 2: ok = db_get_cloud_id(db,susc_id,C_MYSQLDB,&cloud_id,error,&db_fail);
					break;
				case 3: ok = db_get_cloud_id(db,susc_id,C_MSSQLDB,&cloud_id,error,&db_fail);
					break;
			}
			if(ok){
				c = list_cloud_find_id(cl,cloud_id);
				t->cloud = c;
				sprintf(send_message,"2susc_id|%s", susc_id);
				task_cloud_send(t,send_message);
			} else {
				if(db_fail)
					task_done(t,ERROR_FATAL);
			}
		} else if(t->step == 4){
			/* Accionamos sobre el CORE */
			if(!db_susc_stop(db,t->data,error,&db_fail))
				task_done(t,ERROR_FATAL);
			else
				task_done(t,"{\"code\":\"214\",\"info\":\"Suscripcion detenida\"}");
		}
		t->step++;	
	} else if(t->status == T_WAITING){
		task_cloud_get(t);
		if(t->status == T_DONE){
			if(t->result_code == 200){
				/* Pasamos al siguiente paso */
				t->status = T_TODO;
			}
		}
	}
}

void task_susc_start(T_task *t, T_db *db,T_list_cloud *cl){
	/* Le indicamos a las distintas nubes que forman parte
 	   de la suscripcion que inicien todos los elementos
	   de la suscripcion */
	int ok;
	char *susc_id;
	char plan_id[30];
	char error[200];
	int db_fail;
	int cloud_id;
	T_cloud *c = NULL;
	char send_message[100];

	if(t->status == T_TODO){
		if(t->step == 0){
			/* Realizamos chequeos previos */
			if(!db_susc_exist(db,t->data,error,&db_fail)){
				if(db_fail)
					task_done(t,ERROR_FATAL);
				else
					task_done(t,error);
			} else 
				if(!db_susc_prepare(db,t->data,3))
					task_done(t,ERROR_FATAL);
		} else if(t->step > 0 && t->step < 4){
			/* Accionamos sobre las nubes */
			strcpy(plan_id,"0");	//BUSCAR EL PLAN_ID CORRECTO EN BASE AL SUSCRIPTION_ID
			switch (t->step){
				case 1: ok = db_get_cloud_id(db,plan_id,C_WEB,&cloud_id,error,&db_fail);
					break;
				case 2: ok = db_get_cloud_id(db,plan_id,C_MYSQLDB,&cloud_id,error,&db_fail);
					break;
				case 3: ok = db_get_cloud_id(db,plan_id,C_MSSQLDB,&cloud_id,error,&db_fail);
					break;
			}
			if(ok){
				c = list_cloud_find_id(cl,cloud_id);
				t->cloud = c;
				sprintf(send_message,"3susc_id|%s", susc_id);
				task_cloud_send(t,send_message);
			} else {
				if(db_fail)
					task_done(t,ERROR_FATAL);
			}
		} else if(t->step == 4){
			/* Accionamos sobre el CORE */
			if(!db_susc_start(db,t->data,error,&db_fail))
				task_done(t,ERROR_FATAL);
			else
				task_done(t,"{\"code\":\"215\",\"info\":\"Suscripcion iniciada\"}");
		}
		t->step++;	
	} else if(t->status == T_WAITING){
		task_cloud_get(t);
		if(t->status == T_DONE){
			if(t->result_code == 200){
				/* Pasamos al siguiente paso */
				t->status = T_TODO;
			}
		}
	}
}


/*************	TASK USER  ***********************/

void task_user_list(T_task *t, T_db *db){
	/* Obtiene el listado de usuarios */
	MYSQL_RES *db_r;
	char *aux=NULL;

	logs_write(t->logs,L_DEBUG,"task_user_list","");
	if(!db_user_list(db,&db_r)){
		logs_write(t->logs,L_ERROR,"DB ERROR","");
		task_done(t,ERROR_FATAL);
	} else {
		json_user_list(&aux,db_r);
		task_done(t,aux);
	}
	free(aux);
}

void task_user_show(T_task *t, T_db *db){
	/* Obtiene la informacion de un usuario */
	MYSQL_RES *db_r;
	char *aux=NULL;
	char error[200];
	int db_fail;

	printf("TASK_USER_SHOW\n");
	if(!db_user_show(db,t->data,&db_r,error,&db_fail)){
		if(db_fail)
			task_done(t,ERROR_FATAL);
		else
			task_done(t,error);
	} else {
		json_user_show(&aux,db_r);
		task_done(t,aux);
	}
	free(aux);
}

void task_user_add(T_task *t, T_db *db){
	/* Agrega un usuario */
	char result[200];
	int db_fail;
	
	if(db_user_add(db,t->data,result,&db_fail)){
		task_done(t,"{\"code\":\"221\",\"info\":\"usuario agregado\"}");
	} else {
		if(db_fail)
			task_done(t,ERROR_FATAL);
		else
			task_done(t,result);
	}
}


int task_user_mod(T_task *t, T_db *db){
	/* Modifica un usuario */
	char result[100];
	int db_fail;

	if(db_user_mod(db,t->data,result,&db_fail)){
		task_done(t,"{\"code\":\"223\",\"info\":\"usuario modificado\"}");
	} else {
		if(db_fail)
			task_done(t,ERROR_FATAL);
		else
			task_done(t,result);
	}
}

int task_user_del(T_task *t, T_db *db, T_list_cloud *cl){
	/* Elimina un usuario y todos sus datos
	   en nubes */
	int db_fail;
	char error[200];
	MYSQL_RES *result;
	MYSQL_ROW row;

	if(t->status == T_TODO){
		/* Eliminamos sus suscripciones */
		if(db_susc_list(db,t->data,&result,error,&db_fail)){
			/* Eliminamos la primer suscripcion del listado */
			result = mysql_store_result(db->con);
			if(mysql_num_rows(result)> 0){
				/* Quedan suscripciones */
				row = mysql_fetch_row(result);
				dictionary_add(t->data,"susc_id",row[0]);
				task_susc_del(t,db,cl);
			} else {
				/* Ya no tiene suscripciones. Procedemos
 				   a eliminar el usuario */
				db_user_del(db,t->data,&result,error,&db_fail);
				if(db_fail){
					task_done(t,ERROR_FATAL);
				} else {
					task_done(t,"{\"code\":\"222\",\"info\":\"Usuario eliminado\"}");
				}
			}
		} else {
			task_done(t,ERROR_FATAL);
		}
	} else if(t->status == T_WAITING){
		task_susc_del(t,db,cl);
		if(t->status == T_DONE){
			dictionary_remove(t->data,"susc_id");
			t->status == T_TODO;
			t->step=0;
		}
	}
}

void task_user_stop(T_task *t, T_db *db, T_list_cloud *cl){
	int db_fail;
	MYSQL_RES *result;
	char error[200];
	MYSQL_ROW row;

	if(t->status == T_TODO){
		/* Detenemos sus suscripciones */
		if(db_susc_list(db,t->data,&result,error,&db_fail)){
			/*Detenemos la primer suscripcion del listado */
			result = mysql_store_result(db->con);
			if(mysql_num_rows(result)> 0){
				/* Quedan suscripciones por detener */
				row = mysql_fetch_row(result);
				dictionary_add(t->data,"susc_id",row[0]);
				task_susc_stop(t,db,cl);
			} else {
				/* Ya no tiene suscripciones por detener. Procedemos
 				   a detener el usuario */
				db_user_stop(db,t->data,error,&db_fail);
				if(db_fail){
					task_done(t,ERROR_FATAL);
				} else {
					task_done(t,"{\"code\":\"224\",\"info\":\"Usuario inhabilitado\"}");
				}
			}
		} else {
			task_done(t,ERROR_FATAL);
		}
	} else if(t->status == T_WAITING){
		task_susc_del(t,db,cl);
		if(t->status == T_DONE){
			dictionary_remove(t->data,"susc_id");
			t->status == T_TODO;
			t->step=0;
		}
	}
}

void task_user_start(T_task *t, T_db *db, T_list_cloud *cl){
	int db_fail;
	MYSQL_RES *result;
	char error[200];
	MYSQL_ROW row;

	if(t->status == T_TODO){
		/* Detenemos sus suscripciones */
		if(db_susc_list(db,t->data,&result,error,&db_fail)){
			/*Iniciamos la primer suscripcion del listado */
			result = mysql_store_result(db->con);
			if(mysql_num_rows(result)> 0){
				/* Quedan suscripciones por iniciar */
				row = mysql_fetch_row(result);
				dictionary_add(t->data,"susc_id",row[0]);
				task_susc_start(t,db,cl);
			} else {
				/* Ya no tiene suscripciones por iniciar. Procedemos
 				   a iniciar el usuario */
				db_user_start(db,t->data,error,&db_fail);
				if(db_fail){
					task_done(t,ERROR_FATAL);
				} else {
					task_done(t,"{\"code\":\"224\",\"info\":\"Usuario habilitado\"}");
				}
			}
		} else {
			task_done(t,ERROR_FATAL);
		}
	} else if(t->status == T_WAITING){
		task_susc_start(t,db,cl);
		if(t->status == T_DONE){
			dictionary_remove(t->data,"susc_id");
			t->status == T_TODO;
			t->step=0;
		}
	}
}

void task_json_result(T_task *t, char **result){
	/* Aloca memoria y concatena la respuesta del task que ya deberia venir
 	 * en formato json agregando dicho contenido en un strig que se entregara
 	 * a los clientes tambien en forma json */

	char aux[20];

	task_print_status(t, aux);
	if(t->status == T_DONE){
		*result=(char *)realloc(*result,strlen(t->result) + 200);
		sprintf(*result,"{\"taskid\":\"%s\",\"status\":\"%s\",\"data\":%s}",t->id,aux,t->result);
	} else {
		*result=(char *)realloc(*result,200);
		sprintf(*result,"{\"taskid\":\"%s\",\"status\":\"%s\"}",t->id,aux);
	}
}

/********************    TASK CLOUD   *****************************/

void task_cloud_show(T_task *t, T_list_cloud *cl){
	/* Retorna el estado de una nube en particular */
	/* Los datos podemos tomarlos directamente de la estructura */

	char *aux;

	if(json_cloud_show(&aux,dictionary_get(t->data,"cloud_id"),cl)){
		task_done(t,aux);
		free(aux);
	} else {
		task_done(t,"{\"code\":\"200\",\"info\":\"Nube inexistente\"}");
	}
		
}

void task_cloud_list(T_task *t, T_db *db, T_list_cloud *cl){
	/* Lista las nubes existentes con alguna informacion sobre ellas */

	MYSQL_RES *db_r;
	char *aux=NULL;

	printf("ENTROOOOOOOOOOOOOOO\n");
	logs_write(t->logs,L_DEBUG,"task_cloud_list","");
	if(!db_cloud_list(db,&db_r)){
		logs_write(t->logs,L_ERROR,"DB ERROR","");
		task_done(t,ERROR_FATAL);
	} else {
		json_cloud_list(&aux,db_r,cl);
		task_done(t,aux);
	}
	free(aux);
}

int task_cloud_send(T_task *t, char *send_message){
	/* Envia una solicitud a la nube. Se guarda el task id en la estructura del task. */
	/* Si no puede contactar a la nube retorna 0. Caso contrario retorna 1. */
 	/* Cambia el estado del task a T_WAITING si pudo contactar a la nube. */

	char *rcv_message = NULL;
	uint32_t rcv_message_size = 0;
	char value[100];
	int pos=1;

	// pasamos el tamano del mensaje +1 para incluir el '\0'
	if(cloud_send_receive(t->cloud,send_message,strlen(send_message)+1,&rcv_message,&rcv_message_size)){
		parce_data(rcv_message,'|',&pos,value);
		if(rcv_message[0] == '1'){
			/* La nube acepto la tarea */
			dictionary_add(t->data,"c_task_id",value);
			t->status = T_WAITING;
			return 1;
		}
	}
	task_done(t,ERROR_FATAL);
	return 0;
}

int task_cloud_get(T_task *t){
	/* Solicita a la nube el estado de un task
 	 * y el resultado del mismo. Cambia el estado del
 	 * task a T_DONE si se obtuvo resultado de la nube
 	 * o si la consulta a la misma falla. */
	char send_message[100];
	char *rcv_message = NULL;
	char result_code[10];
	int pos = 0;
	uint32_t rcv_message_size = 0;

	sprintf(send_message,"t%s",dictionary_get(t->data,"c_task_id"));
	if(cloud_send_receive(t->cloud,send_message,strlen(send_message)+1,&rcv_message,&rcv_message_size)){
		if(rcv_message[0] == '0'){
			/* El task no ha terminado del lado de la nube */
		} else if(rcv_message[0] == '1'){
			parce_data(rcv_message + 1,'|',&pos,result_code);
			printf("TASK_CLOUD_GET: %s\n",result_code);
			t->result_code = atoi(result_code);
			task_done(t,rcv_message+5);
		} else {
			/* deberia ser un 2. Entonces task no existe en la nube */
			task_done(t,ERROR_TASK_CLOUD);
		}
	} else {
		//Fallo la conexion a la nube
		task_done(t,ERROR_CLOUD);
	}
	free(rcv_message);
}

/*************	TASK SITES  ***********************/

int task_site_list(T_task *t, T_db *db){
	/* Solicita al Controller el listado de sitios de una
 	 * suscripcion. El Controller ya lo retorna en formato json */
	char send_message[100];
	int db_fail;
	char message[200];

	printf("TASK_SITE_LIST\n");
	if(t->status == T_TODO){
		if(db_susc_exist(db,t->data,message,&db_fail)){
			sprintf(send_message,"lsusc_id|%s", dictionary_get(t->data,"susc_id"));
			task_cloud_send(t,send_message);
		} else {
			if(db_fail)
				task_done(t,ERROR_FATAL);
			else
				task_done(t,message);
		}
	} else if(t->status == T_WAITING){
		task_cloud_get(t);
	}
}

int task_site_show(T_task *t, T_db *db){
	/* Solicita al Controller los datos de un sitio.
 	 El Controller ya lo retorna en formato json */
	char send_message[100];
	int db_fail;
	char error[200];

	/* Verificamos que el susc_id sea del cliente user_id */

	if(t->status == T_TODO){
		if(db_susc_exist(db,t->data,error,&db_fail)){
			sprintf(send_message,"ssite_id|%s|susc_id|%s",
				dictionary_get(t->data,"site_id"),
				dictionary_get(t->data,"susc_id"));
			task_cloud_send(t,send_message);
		} else {
			if(db_fail)
				task_done(t,ERROR_FATAL);
			else
				task_done(t,error);
		}
	} else if(t->status == T_WAITING){
		task_cloud_get(t);
	}
}


int task_site_add(T_task *t, T_db *db){
	/* Solicita a la nube web agregar un sitio nuevo.
 	 * la nube ya retorna en formato json el resultado */
	char send_message[100];
	int db_fail;
	char error[200];

	if(t->status == T_TODO){
		/* Analizamos cantidad de sitios permitidos*/
		if(!db_accept_add_site(db,t->data,error,&db_fail)){
			if(db_fail)
				task_done(t,ERROR_FATAL);
			else
				task_done(t,error);
		} else {
			/* Enviamos la accion a la nube */
			sprintf(send_message,"aname|%s|susc_id|%s",
				dictionary_get(t->data,"name"),
				dictionary_get(t->data,"susc_id"));
			task_cloud_send(t,send_message);
		}

	} else if(t->status == T_WAITING){
		task_cloud_get(t);
	}
}

int task_site_del(T_task *t, T_db *db){
	/* Elimina fisica y logicamente un sitio */
	char send_message[100];
	int db_fail;
	char error[200];

	if(t->status == T_TODO){
		if(db_susc_exist(db,t->data,error,&db_fail)){
			sprintf(send_message,"dsusc_id|%s|site_id|%s",
				dictionary_get(t->data,"susc_id"),
				dictionary_get(t->data,"site_id"));
			task_cloud_send(t,send_message);
		} else {
			if(db_fail)
				task_done(t,ERROR_FATAL);
			else
				task_done(t,error);
		}
	} else if(t->status == T_WAITING){
		task_cloud_get(t);
	}
}

void task_site_mod(T_task *t, T_db *db){
	/* Realiza modificaciones sobre un sitio */
	char error[200];
	int db_fail;
	char send_message[200];

	if(t->status == T_TODO){
		/* Verificamos que la suscripcion
 		   sea del usuario */
		if(db_susc_exist(db,t->data,error,&db_fail)){
			sprintf(send_message,"msusc_id|%s|site_id|%s",
			dictionary_get(t->data,"susc_id"),
			dictionary_get(t->data,"site_id"));
			task_cloud_send(t,send_message);
		} else {
			if(db_fail)
				task_done(t,ERROR_FATAL);
			else
				task_done(t,error);
		}
	} else if(t->status == T_WAITING){
		task_cloud_get(t);
	}
}


void task_site_stop(T_task *t, T_db *db){
	/* Le indica a la nube detener un sitio */
	char error[200];
	int db_fail;
	char send_message[200];

	if(t->status == T_TODO){
		/* Verificamos que la suscripcion
 		   sea del usuario */
		if(db_susc_exist(db,t->data,error,&db_fail)){
			sprintf(send_message,"ksusc_id|%s|site_id|%s",
			dictionary_get(t->data,"susc_id"),
			dictionary_get(t->data,"site_id"));
			task_cloud_send(t,send_message);
		} else {
			if(db_fail)
				task_done(t,ERROR_FATAL);
			else
				task_done(t,error);
		}
	} else if(t->status == T_WAITING){
		task_cloud_get(t);
	}
}

void task_site_start(T_task *t, T_db *db){
	/* Le indica a la nube arrancar un sitio */
	char error[200];
	int db_fail;
	char send_message[200];

	if(t->status == T_TODO){
		/* Verificamos que la suscripcion
 		   sea del usuario */
		if(db_susc_exist(db,t->data,error,&db_fail)){
			sprintf(send_message,"esusc_id|%s|site_id|%s",
			dictionary_get(t->data,"susc_id"),
			dictionary_get(t->data,"site_id"));
			task_cloud_send(t,send_message);
		} else {
			if(db_fail)
				task_done(t,ERROR_FATAL);
			else
				task_done(t,error);
		}
	} else if(t->status == T_WAITING){
		task_cloud_get(t);
	}
}

/**************************************
 * 		TASK FTP
 **************************************/
void task_ftp_list(T_task *t, T_db *db){
	if(t->status == T_TODO){
		if(db_susc_exist(db,t->data,error,&db_fail)){
			sprintf(send_message,"bsite_id|%s",
			dictionary_get(t->data,"site_id"));
			task_cloud_send(t,send_message);
		} else {
			if(db_fail)
				task_done(t,ERROR_FATAL);
			else
				task_done(t,error);
		}
	} else if(t->status == T_WAITING){
		task_cloud_get(t);
	}
}

void task_ftp_add(T_task *t, T_db *db){
	if(t->status == T_TODO){
		if(db_susc_exist(db,t->data,error,&db_fail)){
			sprintf(send_message,"csite_id|%s|user_id|%s|passwd|%s",
			dictionary_get(t->data,"site_id"),
			dictionary_get(t->data,"user_id"),
			dictionary_get(t->data,"passwd"));
			task_cloud_send(t,send_message);
		} else {
			if(db_fail)
				task_done(t,ERROR_FATAL);
			else
				task_done(t,error);
		}
	} else if(t->status == T_WAITING){
		task_cloud_get(t);
	}
}

void task_ftp_del(T_task *t, T_db *db){
	if(t->status == T_TODO){
		if(db_susc_exist(db,t->data,error,&db_fail)){
			sprintf(send_message,"fftp_id|%s",
			dictionary_get(t->data,"ftp_id"));
			task_cloud_send(t,send_message);
		} else {
			if(db_fail)
				task_done(t,ERROR_FATAL);
			else
				task_done(t,error);
		}
	} else if(t->status == T_WAITING){
		task_cloud_get(t);
	}
}

void task_ftp_mod(T_task *t, T_db *db){
	if(t->status == T_TODO){
		if(db_susc_exist(db,t->data,error,&db_fail)){
			sprintf(send_message,"gftp_id|%s|passwrd|%s",
			dictionary_get(t->data,"ftp_id"),
			dictionary_get(t->data,"passwd"));
			task_cloud_send(t,send_message);
		} else {
			if(db_fail)
				task_done(t,ERROR_FATAL);
			else
				task_done(t,error);
		}
	} else if(t->status == T_WAITING){
		task_cloud_get(t);
	}
}

void task_hw_server_list(T_task *t, T_list_cloud *cl){
	char send_message[200];

	if(t->status == T_TODO){
		task_assign_cloud(t,cl);
		sprintf(send_message,"L");
		task_cloud_send(t,send_message);
	} else if(t->status == T_WAITING){
		task_cloud_get(t);
	}
}

void task_hw_server_show(T_task *t, T_list_cloud *cl){
	char send_message[200];

	if(t->status == T_TODO){
		task_assign_cloud(t,cl);
		sprintf(send_message,"Sserver_id|%s",
		dictionary_get(t->data,"server_id"));
		task_cloud_send(t,send_message);
	} else if(t->status == T_WAITING){
		task_cloud_get(t);
	}
}

void task_hw_server_stop(T_task *t, T_list_cloud *cl){
	char send_message[200];

	if(t->status == T_TODO){
		task_assign_cloud(t,cl);
		sprintf(send_message,"Kserver_id|%s",
		dictionary_get(t->data,"server_id"));
		task_cloud_send(t,send_message);
	} else if(t->status == T_WAITING){
		task_cloud_get(t);
	}
}

void task_hw_server_start(T_task *t, T_list_cloud *cl){
	char send_message[200];

	if(t->status == T_TODO){
		task_assign_cloud(t,cl);
		sprintf(send_message,"Eserver_id|%s",
		dictionary_get(t->data,"server_id"));
		task_cloud_send(t,send_message);
	} else if(t->status == T_WAITING){
		task_cloud_get(t);
	}
}

/***************************************************/

void task_run(T_task *t, T_db *db, T_list_cloud *cl){
	T_cloud *c;
	char *valor;
	char error[200];
	//char plan_id[30];
	int cloud_id;
	int db_fail;

	if(t->type <= T_SUSC_DEL ){
		/* No son acciones sobre una nube */
		switch(t->type){
			/* USERS */
			case T_USER_LIST:	task_user_list(t,db); break;
			case T_USER_SHOW:	task_user_show(t,db); break;
			case T_USER_ADD:	task_user_add(t,db); break;
			case T_USER_MOD:	task_user_mod(t,db); break;
			case T_USER_DEL:	task_user_del(t,db,cl); break;
			case T_USER_STOP:	task_user_stop(t,db,cl); break;
			case T_USER_START:	task_user_start(t,db,cl); break;
	
			/* SUSCRIPTION */
			case T_SUSC_LIST:	task_susc_list(t,db); break;
			case T_SUSC_SHOW:	task_susc_show(t,db,cl); break;
			case T_SUSC_ADD:	task_susc_add(t,db,cl); break;
			case T_SUSC_MOD:	task_susc_mod(t,db); break;
			case T_SUSC_DEL:	task_susc_del(t,db,cl); break;
			case T_SUSC_STOP:	task_susc_stop(t,db,cl); break;
			case T_SUSC_START:	task_susc_start(t,db,cl); break;

			case T_CLOUD_LIST:	task_cloud_list(t,db,cl); break;
			case T_CLOUD_SHOW:	task_cloud_show(t,cl); break;
		}
	} else {
		/* Son acciones sobre alguna nube */
		if(t->cloud == NULL){
			/* Averiguamos la nube */
			if( t->type <= T_SITE_DEL){
				/* Acciones sobre una nube web */
				valor = dictionary_get(t->data,"susc_id");
				//strcpy(plan_id,"0");	//BUSCAR EL PLAN ID CORRESPONDIENTE AL SUSCRIPTION ID
				if(! db_get_cloud_id(db,valor,C_WEB,&cloud_id,error,&db_fail)){
					if(db_fail)
						task_done(t,ERROR_FATAL);
					else
						task_done(t,"{\"code\":\"312\",\"info\":\"Suscripcion no posee nube indicada\"}");
					return;
				} else {
					c = list_cloud_find_id(cl,cloud_id);
					t->cloud = c;
				}
			}
		}
		/* Una vez hubicada la nube... ejecutamos la tarea */
		switch(t->type){

			case T_SITE_LIST: task_site_list(t,db); break;
			case T_SITE_SHOW: task_site_show(t,db); break;
			case T_SITE_ADD: task_site_add(t,db); break;
			case T_SITE_MOD: task_site_mod(t,db); break;
			case T_SITE_DEL: task_site_del(t,db); break;
			case T_SITE_STOP: task_site_stop(t,db); break;
			case T_SITE_START: task_site_start(t,db); break;

			case T_FTP_LIST: task_ftp_list(t,db); break;
			case T_FTP_ADD: task_ftp_add(t,db); break;
			case T_FTP_DEL: task_ftp_del(t,db); break;
			case T_FTP_MOD: task_ftp_mod(t,db); break;

			case T_HW_SERVER_LIST:task_hw_server_list(t,cl); break;
			case T_HW_SERVER_SHOW:task_hw_server_show(t,cl); break;
			case T_HW_SERVER_STOP:task_hw_server_stop(t,cl); break;
			case T_HW_SERVER_START:task_hw_server_start(t,cl); break;
		}
	}
}

/*****************************
	  Cola de Jobs
 ******************************/

void heap_task_init(T_heap_task *h){
	h->first = NULL;
	h->last = NULL;
	h->size = 0;
}

void heap_task_push(T_heap_task *h, T_task *t){
	heap_t_node *new;
	heap_t_node *aux;

	new = (heap_t_node*)malloc(sizeof(heap_t_node));
	new->next = NULL;
	new->data = t;
	h->size++;

	if(h->first == NULL){
		h->first = new;
		h->last = new;
	} else {
		h->last->next = new;
		h->last = new;
	}
}

T_task *heap_task_pop(T_heap_task *h){
	heap_t_node *aux;
	T_task *taux;

	if(h->first != NULL){
		aux = h->first;
		taux = h->first->data;
		h->first = h->first->next;
		if(h->first == NULL)
			h->last = NULL;
		free(aux);
		return taux;
	} else {
		return NULL;
	}
}

unsigned int heap_task_size(T_heap_task *h){
	return h->size;
}

T_task *heap_task_exist(T_heap_task *h, T_taskid id){
	/* indica si el trabajo existe en la cola y retorna
 	 * el puntero al mismo */
	heap_t_node *aux;
	T_task *exist = NULL;
	
	aux = h->first;
	while(exist == NULL && aux!= NULL){
		if(strcmp(task_get_id(aux->data),id) == 0)
			exist = aux->data;
		aux = aux->next;
	}
	return exist;
}

void heap_task_print(T_heap_task *h){

	heap_t_node *aux;

	aux = h->first;
	while(aux!= NULL){
		aux = aux->next;
	}
}

/******************************
 * 		BAG JOB
 ******************************/

void bag_task_init(T_bag_task *b){
	b->first = NULL;
	b->actual = NULL;
	b->last = NULL;
	b->size = 0;
}

void bag_task_add(T_bag_task *b, T_task *t){
	bag_t_node *new;
	bag_t_node *aux;

	new = (bag_t_node*)malloc(sizeof(bag_t_node));
	new->next = NULL;
	new->data = t;
	b->size++;

	if(b->first == NULL){
		b->first = new;
		b->last = new;
	} else {
		b->last->next = new;
		b->last = new;
	}
}

T_task *bag_site_remove(T_bag_task *b){
	bag_t_node *prio;
	bag_t_node *aux;
	T_task *element = NULL;

	if(b->actual != NULL){
		aux = b->first;
		prio = NULL;
		while(aux != b->actual){
			prio = aux;
			aux = aux->next;
		}
		if(prio == NULL){
			b->first = aux->next;
		} else {
			prio->next = aux->next;
		}
		if(aux == b->last){
			b->last = prio;
		}
		b->actual = aux->next;
		element = aux->data;
		free(aux);
	}
	return element;
}

T_task *bag_task_pop(T_bag_task *b, T_taskid *id){
	/* Retorna el task buscado por su id.
 	 * si no existe retorna NULL. Si existe
 	 * no solo lo retorna sino que lo elimina
 	 * de la bolsa */
	int exist = 0;
	T_task *taux = NULL;

	b->actual = b->first;
	while((b->actual != NULL) && !exist){
		exist = (strcmp(task_get_id(b->actual->data),(char *)id)==0);
		if((!exist && (b->actual != NULL))){
			b->actual = b->actual->next;
		}
	}
	if(exist)
		taux = bag_site_remove(b);
	return taux;
}

unsigned int bag_task_size(T_bag_task *b){
	return b->size;
}

void bag_task_print(T_bag_task *b){

	bag_t_node *aux;

	aux = b->first;
	while(aux!= NULL){
		aux = aux->next;
	}
}

