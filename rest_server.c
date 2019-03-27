#include "rest_server.h"

int check_plan_add(T_dictionary *data, char *result){
	CHECK_VALID_ID(plan_id,plan)
	return 1;
}

int check_user_show(T_dictionary *data, char *result){
	CHECK_VALID_ID(user_id,usuario)
	return 1;
}

int check_susc_show(T_dictionary *data, char *result){
	CHECK_VALID_ID(user_id,usuario)
	CHECK_VALID_ID(susc_id,subscripcion)
	return 1;
}

int check_site_show(T_dictionary *data, char *result){
	CHECK_VALID_ID(user_id,usuario)
	CHECK_VALID_ID(susc_id,subscripcion)
	CHECK_VALID_ID(susc_id,sitio)
	return 1;
}

int check_site_list(T_dictionary *data, char *result){
	CHECK_VALID_ID(user_id,usuario)
	CHECK_VALID_ID(susc_id,suscripcion)
	return 1;
}

int check_user_add(T_dictionary *data, char *result){
	/* Verifica que los parametros esten todos y que
 	 * tengan un formato correcto. En *result retorna
 	 * el error en formato json de ser necesario. */
	if(!valid_user_name(dictionary_get(data,"name"))){
		strcpy(result,"{\"task\":\"\",\"status\":\"ERROR\",\"data\":\"Nombre usuario invalido\"}");
		return 0;
	}
	printf("LLEGOOOOO\n");
	if(!valid_passwd(dictionary_get(data,"pass"))){
		strcpy(result,"{\"task\":\"\",\"status\":\"ERROR\",\"data\":\"Password invalida\"}");
		return 0;
	}
	if(!valid_email(dictionary_get(data,"email"))){
		strcpy(result,"{\"task\":\"\",\"status\":\"ERROR\",\"data\":\"email invalido\"}");
		return 0;
	}
	return 1;
}

int check_user_mod(T_dictionary *data, char *result){
	/* Verifica que los parametros esten todos y que
 	 * tengan un formato correcto. En *result retorna
 	 * el error en formato json de ser necesario */
	CHECK_VALID_ID(user_id,usuario)
	if(!valid_user_name(dictionary_get(data,"name"))){
		strcpy(result,"{\"task\":\"\",\"status\":\"ERROR\",\"data\":\"Nombre usuario invalido\"}");
		return 0;
	}
	if(!valid_passwd(dictionary_get(data,"pass"))){
		strcpy(result,"{\"task\":\"\",\"status\":\"ERROR\",\"data\":\"Password invalida\"}");
		return 0;
	}
	if(!valid_email(dictionary_get(data,"email"))){
		strcpy(result,"{\"task\":\"\",\"status\":\"ERROR\",\"data\":\"email invalido\"}");
		return 0;
	}
	return 1;
}

int check_user_del(T_dictionary *data, char *result){
	CHECK_VALID_ID(user_id,usuario)
	return 1;
}

int check_susc_add(T_dictionary *data, char *result){
	CHECK_VALID_ID(plan_id,plan)
	return 1;
}

int check_susc_mod(T_dictionary *data, char *result){
	CHECK_VALID_ID(susc_id,suscripcion)
	return 1;
}

int check_susc_del(T_dictionary *data, char *result){
	CHECK_VALID_ID(susc_id,suscripcion)
	return 1;
}

int check_site_mod(T_dictionary *data, char *result){
	CHECK_VALID_ID(susc_id,suscripcion)
	CHECK_VALID_ID(site_id,sitio)
	return 1;
}

int check_site_add(T_dictionary *data, char *result){
	CHECK_VALID_ID(susc_id,suscripcion)
	return 1;
}

int check_site_del(T_dictionary *data, char *result){
	CHECK_VALID_ID(susc_id,suscripcion)
	CHECK_VALID_ID(site_id,sitio)
	return 1;
}

int check_cloud_show(T_dictionary *data, char *result){
	CHECK_VALID_ID(cloud_id,nube)
	return 1;
}

int check_ftp_list(T_dictionary *data, char *result){
	CHECK_VALID_ID(susc_id,suscripcion)
	CHECK_VALID_ID(site_id,sitio)
	return 1;
}

int check_ftp_del(T_dictionary *data, char *result){
	CHECK_VALID_ID(susc_id,suscripcion)
	CHECK_VALID_ID(site_id,sitio)
	CHECK_VALID_ID(ftp_id,usuario ftp)
	return 1;
}

int check_ftp_add(T_dictionary *data, char *result){
	CHECK_VALID_ID(susc_id,suscripcion)
	CHECK_VALID_ID(site_id,sitio)
	if(!valid_passwd(dictionary_get(data,"passwd"))){
		strcpy(result,"{\"task\":\"\",\"status\":\"ERROR\",\"data\":\"Password invalida\"}");
		return 0;
	}
	if(!valid_ftp_name(dictionary_get(data,"name"))){
		strcpy(result,"{\"task\":\"\",\"status\":\"ERROR\",\"data\":\"nombre usuario invalido\"}");
		return 0;
	}
	return 1;
}

int check_ftp_mod(T_dictionary *data, char *result){
	CHECK_VALID_ID(susc_id,suscripcion)
	CHECK_VALID_ID(site_id,sitio)
	CHECK_VALID_ID(ftp_id,usuario ftp)
	return 1;
}


void rest_server_add_task(T_rest_server *r, T_task *j){

	pthread_mutex_lock(&(r->mutex_heap_task));
		heap_task_push(&(r->tasks_todo),j);
		/* Para debug imprimimis la lista hasta el momento */
		//heap_task_print(&(r->tasks_todo));
	pthread_mutex_unlock(&(r->mutex_heap_task));
}

uint32_t rest_server_num_tasks(T_rest_server *r){
	printf("cantidad tareas: %u,%u\n",heap_task_size(&(r->tasks_todo)), bag_task_size(&(r->tasks_done)));
	return (heap_task_size(&(r->tasks_todo)) + bag_task_size(&(r->tasks_done)));
}

void rest_server_url_error(char *result, int *ok){
	strcpy(result,"{\"task\":\"\",\"status\":\"ERROR\",\"data\":\"api call invalid\"}");
	*ok=0;
}

void *rest_server_purge_done(void *param){
	/* Se encarga de purgar cada 10 segundos la estructura
	 * de tareas finalizadas */

	T_rest_server *r= (T_rest_server *)param;
	while(1){
		sleep(10);
		pthread_mutex_lock(&(r->mutex_bag_task));
			bag_task_timedout(&(r->tasks_done),
			config_task_done_time(r->config));
		pthread_mutex_unlock(&(r->mutex_bag_task));
	}
}

void *rest_server_do_task(void *param){
	T_rest_server *r= (T_rest_server *)param;

	while(1){
		//sleep(2);
		pthread_mutex_lock(&(r->mutex_heap_task));
			r->runningTask = heap_task_pop(&(r->tasks_todo));
		pthread_mutex_unlock(&(r->mutex_heap_task));
		if(r->runningTask != NULL){
			/* Si la tarea hace mas de un minuto que esta en cola
 			 * vence por timeout */
			if(60 < difftime(time(NULL),task_get_time(r->runningTask)))
				task_done(r->runningTask,"{\"status\":\"DONE\",\"data\":\"Task time Out\"}");
			else {
				//printf("DOOO\n");
				task_run(r->runningTask,r->db,r->clouds);
				//printf("fin DOOO\n");
			}

			if(task_get_status(r->runningTask) == T_DONE){
				pthread_mutex_lock(&(r->mutex_bag_task));
					bag_task_add(&(r->tasks_done),r->runningTask);
					r->runningTask = NULL;
				pthread_mutex_unlock(&(r->mutex_bag_task));
			} else {
				pthread_mutex_lock(&(r->mutex_heap_task));
					heap_task_push(&(r->tasks_todo),r->runningTask);
					r->runningTask = NULL;
				pthread_mutex_unlock(&(r->mutex_heap_task));
			}
		}
	}
}

static int send_page(struct MHD_Connection *connection, const char *page){
	int ret;
	struct MHD_Response *response;

	response = MHD_create_response_from_buffer(strlen(page), (void *)page,
			MHD_RESPMEM_PERSISTENT);
	if (!response)
		return MHD_NO;

	MHD_add_response_header(response,"Access-Control-Allow-Origin","*");
	MHD_add_response_header(response,"Access-Control-Allow-Methods","POST, GET, DELETE");
	MHD_add_response_header(response,"Access-Control-Allow-Headers","Access-Control-Allow-Origin");

	ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
	MHD_destroy_response (response);

	return ret;
}

void rest_server_get_task(T_rest_server *r, T_taskid *taskid, char **message, unsigned int *message_size){
	/* Retorna en formato json el resultado de la finalizacion de un task */
	T_task *task = NULL;
	char status[30];
	unsigned int total_size;

	pthread_mutex_lock(&(r->mutex_bag_task));
	pthread_mutex_lock(&(r->mutex_heap_task));

	/* Buscamos en la bolsa de tareas finalizadas. */
	bag_task_print(&(r->tasks_done));
	//heap_task_print(&(r->tasks_todo));
	task = bag_task_pop(&(r->tasks_done),taskid);
	if(task == NULL){
		/* Buscamos entonces en la cola de pendientes. En este caso
 		   se retorna una copia del task pero no se quita de la cola */
		task = heap_task_exist(&(r->tasks_todo),(char *)taskid);
	}
	if(task == NULL){
		/* Verificamos si esta actualmente corriendo */
		task = r->runningTask;
	}
	if(task == NULL){
		sprintf(*message,"{\"taskid\":\"%s\",\"status\":\"INEXIST\"}",taskid);
	} else {
		/* Armamos en message el resultado de la terea */
		//task_print_status(task,status);
		printf("REST_SERVER_GET_TASK: json\n");
		task_json_result(task,message);
		printf("REST_SERVER_GET_TASK: json fin\n");
		/* Destruimos el task si este ya a finalizado */
		if(task_get_status(task) > T_WAITING)
			task_destroy(&task);
	}

	pthread_mutex_unlock(&(r->mutex_heap_task));
	pthread_mutex_unlock(&(r->mutex_bag_task));
}

static int handle_POST(struct MHD_Connection *connection,
			const char *url,
			struct connection_info_struct *con_info){
	int pos=1;
	int ok=1;
	char value[100];
	char *result = (char *)malloc(TASKRESULT_SIZE);
	unsigned int size_result = TASKRESULT_SIZE;
	T_task *task=NULL;
	T_taskid *taskid;

	printf("Handle_post ENTRO\n");

	/* El token de momento lo inventamos
 	   pero deberia venir en el header del mensaje */
	T_tasktoken token;
	random_token(token);

	/* Le pasamos el puntero del diccionario del
 	   parametro con_info a la variable task. OJO
	   que entonces ya no es responsabilidad del metodo
	   eliminar la estructura de diccionario. Sino que pasa
	   a ser responsabilidad del task. */

	parce_data((char *)url,'/',&pos,value);
	task = (T_task *)malloc(sizeof(T_task));
	task_init(task,&token,con_info->data,&logs);

	/* PARA LOS PLANES */
	if(0 == strcmp("plans",value)){
		parce_data((char *)url,'/',&pos,value);
		if(strlen(value)>0){
			ok=0;
			rest_server_url_error(result,&ok);
		} else {
			if(ok = check_plan_add(con_info->data,result)){
				task_set_type(task,T_PLAN_ADD);
			}
		}
	/* PARA LOS USUARIOS */
	printf("Handle_POST usuarios\n");
	} else if(0 == strcmp("users",value)){
		parce_data((char *)url,'/',&pos,value);
		if(strlen(value)>0){
			dictionary_add(con_info->data,"user_id",value);
			parce_data((char *)url,'/',&pos,value);
			if(strlen(value)>0){
				/* PARA LAS SUSCRIPCIONES DEL USUARIO */
				if(0 == strcmp("susc",value)){
					printf("Handle_POST suscripciones\n");
					parce_data((char *)url,'/',&pos,value);
					if(strlen(value)>0){
						dictionary_add(con_info->data,"susc_id",value);
						parce_data((char *)url,'/',&pos,value);
						if(0 == strcmp("sites",value)){
							printf("Handle_POST sitios\n");
							parce_data((char *)url,'/',&pos,value);
							if(strlen(value)>0){
								dictionary_add(con_info->data,"site_id",value);
								parce_data((char *)url,'/',&pos,value);
								if(strlen(value)>0){
									if(0 == strcmp("ftp_users",value)){
										parce_data((char *)url,'/',&pos,value);
										if(strlen(value)>0){
											/* EDICION FTP */
											dictionary_add(con_info->data,"ftp_id",value);
											if(ok = check_ftp_mod(con_info->data,result))
												task_set_type(task,T_FTP_MOD);
										} else {
											/* ALTA FTP */
											if(ok = check_ftp_add(con_info->data,result))
												task_set_type(task,T_FTP_ADD);
										}
									} else {
										rest_server_url_error(result,&ok);
									}
								} else {
									/* EDICION SITIO */
									if(ok = check_site_mod(con_info->data,result))
										task_set_type(task,T_SITE_MOD);
								}
							} else {
								/* ALTA SITIO */
								if(ok = check_site_add(con_info->data,result))
									task_set_type(task,T_SITE_ADD);
							}
						} else {
							/* EDICION SUSCRIPCIONES */
							dictionary_add(con_info->data,"susc_id",value);
							if(ok = check_susc_mod(con_info->data,result))
								task_set_type(task,T_SUSC_MOD);
						}
					} else {
						/* ALTA SUSCRIPCIONES */
						printf("Handle_POST alta suscripcion\n");
						if(ok = check_susc_add(con_info->data,result))
							task_set_type(task,T_SUSC_ADD);
					}
				} else {
					rest_server_url_error(result,&ok);
				}
			} else {
				/* EDICION DE USUARIO */
				if(ok = check_user_mod(con_info->data,result))
					task_set_type(task,T_USER_MOD);
			}
		} else {
			/* ALTA DE USUARIO */
			if(ok = check_user_add(con_info->data,result))
				task_set_type(task,T_USER_ADD);
		}
	/* CUALQUIER OTRA COSA. ERROR */
	} else {
		strcpy(result,"URL mal ingresada");
		ok=0;
		rest_server_url_error(result,&ok);
	}

	if(ok){
		if(rest_server_num_tasks(&rest_server) < 200 ){
			rest_server_add_task(&rest_server,task);
			sprintf(result,"{\"task\":\"%s\",\"status\":\"TODO\"}",task_get_id(task));
		} else {
			sprintf(result,"{\"task\":\"%s\",\"status\":\"ERROR\",\"data\":\"Superando limite de tareas\"}");
		}
	} else
		task_destroy(&task);
	
	printf("Enviando al cliente REST: %s\n",result);
	send_page (connection,result);
	return ok;
}

static int handle_DELETE(struct MHD_Connection *connection, const char *url){

	char value[100];
	int pos=1;
	int ok=1;
	T_dictionary *data;
	char *result = (char *)malloc(TASKRESULT_SIZE);
	unsigned int size_result = TASKRESULT_SIZE;
	T_task *task=NULL;
	T_taskid *taskid;


	/* El token de momento lo inventamos
	 * pero deberia venir en el header del mensaje */
	T_tasktoken token;
	random_token(token);

	data = malloc(sizeof(T_dictionary));
	dictionary_init(data);
	task = (T_task *)malloc(sizeof(T_task));
	task_init(task,&token,data,&logs);

	/* PARA LOS PLANES */
	parce_data((char *)url,'/',&pos,value);
	if(0 == strcmp("plans",value)){
		parce_data((char *)url,'/',&pos,value);
		if(strlen(value)> 0){
			/* BORRAR PLAN */
			task_set_type(task,T_PLAN_DEL);
		} else {
			rest_server_url_error(result,&ok);
		}
	} else if(0 == strcmp("users",value)){
		parce_data((char *)url,'/',&pos,value);
		if(strlen(value)> 0){
			dictionary_add(data,"user_id",value);
			parce_data((char *)url,'/',&pos,value);
			if(strlen(value)>0){
				if(0 == strcmp("susc",value)) {
					parce_data((char *)url,'/',&pos,value);
					if(strlen(value)>0){
						dictionary_add(data,"susc_id",value);
						parce_data((char *)url,'/',&pos,value);
						if(strlen(value)>0){
							if(0 == strcmp("sites",value)) {
								parce_data((char *)url,'/',&pos,value);
								if(strlen(value)>0) {
									dictionary_add(data,"site_id",value);
									parce_data((char *)url,'/',&pos,value);
									if(strlen(value)>0) {
										if(0 == strcmp("ftp_users",value)) {
											parce_data((char *)url,'/',&pos,value);
											if(strlen(value)>0) {
												dictionary_add(data,"ftp_id",value);
												/* BORRADO FTP */
												if(ok = check_ftp_del(data,result))
													task_set_type(task,T_FTP_DEL);
											} else {
												rest_server_url_error(result,&ok);
											}
										} else {
											rest_server_url_error(result,&ok);
										}
									} else {
										/* BORRADO SITIO */
										if(ok = check_site_del(data,result))
											task_set_type(task,T_SITE_DEL);
									}
								} else {
									rest_server_url_error(result,&ok);
								}
							} else {
								rest_server_url_error(result,&ok);
							}
						} else {
							/* BORRADO SUSCRIPCION */
							if(ok = check_susc_del(data,result))
								task_set_type(task,T_SUSC_DEL);
						}
					} else {
						rest_server_url_error(result,&ok);
					}
				} else {
					rest_server_url_error(result,&ok);
				}
			} else {
				/* BORRAR USUARIO */
				if(ok = check_user_del(data,result))
					task_set_type(task,T_USER_DEL);
			}
		} else {
			rest_server_url_error(result,&ok);
		}
	}

	if(ok){
		if(rest_server_num_tasks(&rest_server) < 200 ){
			rest_server_add_task(&rest_server,task);
			sprintf(result,"{\"task\":\"%s\",\"status\":\"TODO\"}",task_get_id(task));
		} else {
			sprintf(result,"{\"task\":\"%s\",\"status\":\"ERROR\",\"data\":\"Superando limite de tareas\"}");
		}
	} else {
		task_destroy(&task);
	}
	printf("Enviando al cliente REST: %s\n",result);
	send_page (connection,result);
	return ok;
}

static int handle_GET(struct MHD_Connection *connection, const char *url){
	char value[100];
	int pos=1;
	T_dictionary *data;
	char *result = (char *)malloc(TASKRESULT_SIZE);
	unsigned int size_result = TASKRESULT_SIZE;
	T_task *task = NULL;
	T_taskid *taskid;
	int ok=1;	// Resultado a retornar la funcion
	int isTaskStatus =0;	// Cualquier GET excepto el que solisita el estadod e un task se encola.

	/* El token de momento lo inventamos
 	   pero deberia venir en el header del mensaje */
	printf("Handle_get ENTRO\n");

	T_tasktoken token;
	random_token(token);

	data = malloc(sizeof(T_dictionary));
	dictionary_init(data);
	task = (T_task *)malloc(sizeof(T_task));
	task_init(task,&token,data,&logs);


	parce_data((char *)url,'/',&pos,value);
	/* PARA LOS PLANES */
	if(0 == strcmp("plans",value)){
		parce_data((char *)url,'/',&pos,value);
		if(strlen(value)> 0){
			dictionary_add(task_get_data(task),"plan_id",value);
			task_set_type(task,T_PLAN_SHOW);
		} else {
			task_set_type(task,T_PLAN_LIST);
		}

	/* PARA LAS NUBES en general */
	} else if(0 == strcmp("clouds",value)) {
		parce_data((char *)url,'/',&pos,value);
		if(strlen(value)>0){
			dictionary_add(task_get_data(task),"cloud_id",value);
			parce_data((char *)url,'/',&pos,value);
			if(strlen(value)>0){
				// No deberiamos estar aqui. ERROR API
				rest_server_url_error(result,&ok);
			} else {
				if(ok = check_cloud_show(data,result))
					task_set_type(task,T_CLOUD_SHOW);
			}
		} else {
			/* Listado de nubes */
			printf("Handle_GET listado nubes\n");
			task_set_type(task,T_CLOUD_LIST);
		}

	/* PARA LAS NUBES de webhosting */
	} else if(0 == strcmp("webhosting",value)) {
		parce_data((char *)url,'/',&pos,value);
		if(strlen(value)>0){
			dictionary_add(task_get_data(task),"cloud_id",value);
			parce_data((char *)url,'/',&pos,value);
			if(0 == strcmp("servers",value)){
				parce_data((char *)url,'/',&pos,value);
				if(strlen(value)>0){
					dictionary_add(data,"server_id",value);
					parce_data((char *)url,'/',&pos,value);
					if(0 == strcmp("stop",value)){
						task_set_type(task,T_HW_SERVER_STOP);
					} else if(0 == strcmp("start",value)){
						task_set_type(task,T_HW_SERVER_START);
					} else {
						task_set_type(task,T_HW_SERVER_SHOW);
					}
				} else {
					task_set_type(task,T_HW_SERVER_LIST);
				}
			} else if(0 == strcmp("sites",value)) {
				parce_data((char *)url,'/',&pos,value);
				if(strlen(value)>0){
					rest_server_url_error(result,&ok);
				} else {
					printf("PASO LIST SITES\n");
					task_set_type(task,T_HW_SITE_LIST);
				}
			} else {
				rest_server_url_error(result,&ok);
			}
		} else {
			rest_server_url_error(result,&ok);
		}

	/* PARA LOS USUARIOS */
	} else if(0 == strcmp("users",value)) {
		parce_data((char *)url,'/',&pos,value);
		if(strlen(value)>0){
			dictionary_add(task_get_data(task),"user_id",value);
			parce_data((char *)url,'/',&pos,value);
			if(strlen(value)>0){
				/* PARA LAS SUSCRIPCIONES DEL USUARIO */
				if(0 == strcmp("susc",value)) {
					parce_data((char *)url,'/',&pos,value);
					if(strlen(value)>0){
						dictionary_add(task_get_data(task),"susc_id",value);
						parce_data((char *)url,'/',&pos,value);
						if(strlen(value)>0){
							if(0 == strcmp("sites",value)){
								parce_data((char *)url,'/',&pos,value);
								if(strlen(value)>0){
									dictionary_add(task_get_data(task),"site_id",value);
									parce_data((char *)url,'/',&pos,value);
									if(strlen(value)>0){
										if(0 == strcmp("ftp_users",value)){
											parce_data((char *)url,'/',&pos,value);
											if(strlen(value)>0){
												rest_server_url_error(result,&ok);
											} else {
												/* Listado ftp */
												if(ok = check_ftp_list(data,result))
													task_set_type(task,T_FTP_LIST);
											}
										} else {
											rest_server_url_error(result,&ok);
										}
									} else {
										/* Show sitio */
										if(ok = check_site_show(data,result))
											task_set_type(task,T_SITE_SHOW);
									}
								} else {
									/* Listado de sitios */
									if(ok = check_site_list(data,result))
										task_set_type(task,T_SITE_LIST);
								}
							} else {
								rest_server_url_error(result,&ok);
							}
						} else {
							/* Informacion de una suscripcion */
							if(ok = (check_susc_show(data,result)))
								task_set_type(task,T_SUSC_SHOW);
						}
					} else {
						/* Listado suscripciones de un usuario*/
						task_set_type(task,T_SUSC_LIST);
					}
				} else {
					rest_server_url_error(result,&ok);
				}
			} else {
				/* Informacion sobre un usuario */
				if(ok = check_user_show(data,result))
					task_set_type(task,T_USER_SHOW);
			}
		} else {
			/* Listado de usuarios */
			task_set_type(task,T_USER_LIST);
		}

	/* PARA LOS TASK */
	} else if(0 == strcmp("task",value)) {
		/* Se solicita info de un task */
		isTaskStatus =1;
		free(task);
		parce_data((char *)url,'/',&pos,value);
		if(strlen(value)>0){
			rest_server_get_task(&rest_server,(T_taskid *)value,&result,&size_result);
			printf("handle_GET rest_server_get_task\n");
		} else {
			strcpy(result,"{task incorrecto}");
		}

	/* PARA CUALQUIER OTRA COSA */
	} else {
		rest_server_url_error(result,&ok);
	}

	if(ok){
		if(!isTaskStatus){
			if(rest_server_num_tasks(&rest_server) < 200 ){
				sprintf(result,"{\"task\":\"%s\",\"status\":\"TODO\"}",task_get_id(task));
				rest_server_add_task(&rest_server,task);
			} else {
				sprintf(result,"{\"task\":\"%s\",\"status\":\"ERROR\",\"data\":\"Superando limite de tareas\"}");
			}
		}
	} else 
		task_destroy(&task);
	printf("Enviando al cliente REST: %s\n",result);
	send_page (connection, result);
	return ok;
}

static int iterate_post (void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
			const char *filename, const char *content_type,
			const char *transfer_encoding, const char *data, uint64_t off,
			size_t size){

	struct connection_info_struct *con_info = coninfo_cls;

	if(strlen(data)>0){
		printf("iterate_post: Agregamos al diccionario %s->%s\n",(char *)key,(char *)data);
		dictionary_add(con_info->data,(char *)key,(char *)data);
		return MHD_YES;
	} else {
		return MHD_NO;
	}
}

static void request_completed (void *cls, struct MHD_Connection *connection,
			   void **con_cls, enum MHD_RequestTerminationCode toe){

	struct connection_info_struct *con_info = *con_cls;
	if (NULL == con_info)
		return;
	if (con_info->connectiontype == POST){
		MHD_destroy_post_processor (con_info->postprocessor);
	}
	if (con_info->connectiontype != POST){
		dictionary_destroy(&(con_info->data));
	}
	free (con_info);
	*con_cls = NULL;
}

static int answer_to_connection (void *cls, struct MHD_Connection *connection,
				const char *url, const char *method,
				const char *version, const char *upload_data,
				size_t *upload_data_size, void **con_cls){

	if (NULL == *con_cls){
		struct connection_info_struct *con_info;
		con_info = malloc (sizeof (struct connection_info_struct));
		con_info->data = malloc (sizeof (T_dictionary));
		dictionary_init(con_info->data);
		if (NULL == con_info)
			return MHD_NO;
		if (0 == strcmp (method, "POST")){
			con_info->postprocessor = 
				MHD_create_post_processor (connection, POSTBUFFERSIZE,
				iterate_post, (void *) con_info);

			if (NULL == con_info->postprocessor){
				free (con_info);
				return MHD_NO;
			}
			con_info->connectiontype = POST;
		} else {
			con_info->connectiontype = GET;
		}
		*con_cls = (void *) con_info;
		return MHD_YES;
	}
	printf("API_REST url: %s\n",url);

	if (0 == strcmp (method, "GET")){
		handle_GET(connection,url);
		return MHD_YES;
	}

	if (0 == strcmp (method, "DELETE")){
		handle_DELETE(connection,url);
		return MHD_YES;
	}

	if (0 == strcmp (method, "POST")){
		struct connection_info_struct *con_info = *con_cls;
		if (*upload_data_size != 0){
			MHD_post_process (con_info->postprocessor, upload_data,
					*upload_data_size);
			*upload_data_size = 0;
			return MHD_YES;
		} else {
			handle_POST(connection,url,con_info);
			return MHD_YES;
		}
	}
	return send_page(connection, "TODO MAL");
}

void *rest_server_start(void *param){
	
	T_rest_server *r= (T_rest_server *)param;

	r-> rest_daemon = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION | MHD_USE_DEBUG,
			REST_PORT, NULL, NULL, &answer_to_connection, NULL, MHD_OPTION_NOTIFY_COMPLETED,
			request_completed, NULL, MHD_OPTION_END);
}

void rest_server_init(T_rest_server *r, T_db *db, T_list_cloud *c, T_config *config){

	r->db = db;
	r->runningTask = NULL;
	r->config = config;
	r->clouds = c;
	heap_task_init(&(r->tasks_todo));
	bag_task_init(&(r->tasks_done));
	if(0 != pthread_create(&(r->purge_done), NULL, &rest_server_purge_done, r)){
		exit(2);
	}
	if(0 != pthread_create(&(r->do_task), NULL, &rest_server_do_task, r)){
		exit(2);
	}
	if(0 != pthread_create(&(r->thread), NULL, &rest_server_start, r)){
		exit(2);
	}
}

