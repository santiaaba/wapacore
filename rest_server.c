#include "rest_server.h"

int check_data_plan_add(T_dictionary *data, char *result){
	return 1;
}
int check_data_user_add(T_dictionary *data, char *result){
	/* Verificamos existencia parametros */
	if(dictionary_get(data,"name") == NULL){
		strcpy(result,"Falta parametro name");
		return 0;
	}
	if(dictionary_get(data,"pass") == NULL){
		strcpy(result,"Falta parametro pass");
		return 0;
	}
	if(dictionary_get(data,"email") == NULL){
		strcpy(result,"Falta parametro email");
		return 0;
	}
	return 1;
}
int check_data_susc_add(T_dictionary *data, char *result){
	return 1;
}

void rest_server_add_task(T_rest_server *r, T_task *j){
	printf("Agregamos el JOB: %s a la lista\n",task_get_id(j));

	pthread_mutex_lock(&(r->mutex_heap_task));
		printf("ADD_TASK - %s\n",task_get_id(j));
		heap_task_push(&(r->tasks_todo),j);
		/* Para debug imprimimis la lista hasta el momento */
		//heap_task_print(&(r->tasks_todo));
	pthread_mutex_unlock(&(r->mutex_heap_task));

}

void rest_server_error(char *result, int *ok){
	strcpy(result,"{\'task\':\'\',\'stauts\':\'ERROR\',\'data\'=\'api call invalid\'}");
	*ok=0;
}

void *rest_server_do_task(void *param){
	T_task *task;
	T_rest_server *r= (T_rest_server *)param;

	while(1){
		sleep(3);
		pthread_mutex_lock(&(r->mutex_heap_task));
			task = heap_task_pop(&(r->tasks_todo));
			printf("DO_TASK - %s\n",task_get_id(task));
		pthread_mutex_unlock(&(r->mutex_heap_task));
		if(task != NULL){
			task_run(task,r->db);
			pthread_mutex_lock(&(r->mutex_bag_task));
				printf("BAG_TASK - %s\n",task_get_id(task));
				bag_task_add(&(r->tasks_done),task);
				bag_task_print(&(r->tasks_done));
			pthread_mutex_unlock(&(r->mutex_bag_task));
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

	ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
	MHD_destroy_response (response);

	return ret;
}

void rest_server_get_task(T_rest_server *r, T_taskid *taskid, char **message, unsigned int *size){
	T_task *task = NULL;
	char status[30];
	unsigned int total_size;

	pthread_mutex_lock(&(r->mutex_bag_task));
	pthread_mutex_lock(&(r->mutex_heap_task));

	printf("Buscadno TASK ID: %s\n",taskid);
	/* Buscamos en la bolsa de tareas finalizadas. Si la encontramos,
 	   bag_task_print la quita de la estructura bag. */
	bag_task_print(&(r->tasks_done));
	task = bag_task_pop(&(r->tasks_done),taskid);
	printf("PASO %p\n",task);
	if(task == NULL){
		printf("No esta en la bolsa de terminados. La buscamos en la cola de pendientes\n");
		/* Buscamos entonces en la cola de pendientes. En este caso
 		   se retorna una copia del task pero no se quita de la cola */
		task = heap_task_exist(&(r->tasks_todo),(char *)taskid);
		printf("PASO 2 %p\n",task);
	}
	if(task == NULL){
		printf("Tampoco esta en la cola de pendientes\n");
		sprintf(*message,"{'taskid':'%s','status':'INEXIST'}",taskid);
	} else {
		/* Armamos en message el resultado de la terea */
		task_print_status(task,status);
		json_task(status,task_get_id(task),task_get_result(task),message,size);

		/* Destruimos el task si esta ya finalizado */
		if(task_get_status(task) > T_RUNNING){
			printf("ELIMINAMOS EL TASK\n");
			task_destroy(&task);
			printf("TASK ELIMINADO\n");
		}
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
	T_task *task;
	T_taskid *taskid;

	/* El token de momento lo inventamos
 	   pero deberia venir en el header del mensaje */
	T_tasktoken token;
	random_token(token);

	/* Le pasamos el puntero del diccionario del
 	   parametro con_info a la variable task. OJO
	   que entonces ya no es responsabilidad del metodo
	   eliminar la estructura de diccionario. Sino que pasa
	   a ser responsabilidad del task. */

	task = (T_task *)malloc(sizeof(T_task));
	parce_data((char *)url,'/',&pos,value);

	/* PARA LOS PLANES */
	if(0 == strcmp("plans",value)){
		parce_data((char *)url,'/',&pos,value);
		if(strlen(value)>0){
			ok=0;
		
			rest_server_error(result,&ok);
		} else {
			printf("alta de un plan\n");
			if(ok = check_data_plan_add(con_info->data,result))
				task_init(task,&token,T_PLAN_ADD,con_info->data);
		}
	/* PARA LOS USUARIOS */
	} else if(0 == strcmp("users",value)){
		parce_data((char *)url,'/',&pos,value);
		if(strlen(value)>0){
			rest_server_error(result,&ok);
		} else {
			printf("alta de un usuario\n");
			if(ok = check_data_user_add(con_info->data,result))
				task_init(task,&token,T_USER_ADD,con_info->data);
		}
	/* PARA LAS SUSCRIPCIONES */
	} else if(0 == strcmp("suscriptions",value)){
		parce_data((char *)url,'/',&pos,value);
		if(strlen(value)>0){
			rest_server_error(result,&ok);
		} else {
			printf("alta de una suscripsion\n");
			if(ok = check_data_susc_add(con_info->data,result))
				task_init(task,&token,T_SUSC_ADD,con_info->data);
		}
	/* CUALQUIER OTRA COSA. ERROR */
	} else {
		strcpy(result,"URL mal ingresada");
		ok=0;
		rest_server_error(result,&ok);
	}

	if(ok){
		rest_server_add_task(&rest_server,task);
		sprintf(result,"{\"task\":\"%s\",\"status\":\"TODO\"}",task_get_id(task));
	} else {
		printf("No OK : %s\n",result);
		task_destroy(&task);
	}
	send_page (connection,result);
	return ok;
}

static int handle_GET(struct MHD_Connection *connection, const char *url){
	char value[100];
	int pos=1;
	char userid[40];
	T_dictionary *data;
	char *result = (char *)malloc(TASKRESULT_SIZE);
	unsigned int size_result = TASKRESULT_SIZE;
	T_task *task;
	T_taskid *taskid;
	int ok=1;	// Resultado a retornar la funcion
	int isTaskStatus =0;	// Cualquier GET excepto el que solisita el estadod e un task se encola.

	/* El token de momento lo inventamos
 	   pero deberia venir en el header del mensaje */
	T_tasktoken token;
	random_token(token);

	task = (T_task *)malloc(sizeof(T_task));
	parce_data((char *)url,'/',&pos,value);

	/* PARA LOS PLANES */
	if(0 == strcmp("plans",value)){
		parce_data((char *)url,'/',&pos,value);
		if(strlen(value)> 0){
			printf("Info de un plan\n");
			data = malloc(sizeof(T_dictionary));
			dictionary_init(data);
			dictionary_add(data,"id",value);
			task_init(task,&token,T_PLAN_SHOW,data);
		} else {
			printf("listado de planes\n");
			task_init(task,&token,T_PLAN_LIST,NULL);
		}

	/* PARA LOS USUARIOS */
	} else if(0 == strcmp("users",value)) {
		parce_data((char *)url,'/',&pos,value);
		if(strlen(value)>0){
			strcpy(userid,value);
			parce_data((char *)url,'/',&pos,value);
			if(strlen(value)>0){
				/* PARA LAS SUSCRIPCIONES DEL USUARIO */
				if(0 == strcmp("suscriptions",value)) {
					parce_data((char *)url,'/',&pos,value);
					if(strlen(value)>0){
						printf("info de suscripcion\n");
						data = malloc(sizeof(T_dictionary));
						dictionary_init(data);
						dictionary_add(data,"userid",userid);
						dictionary_add(data,"suscripid",value);
						task_init(task,&token,T_SUSC_SHOW,data);
					} else {
						printf("listado de suscripciones del usuario\n");
						task_init(task,&token,T_SUSC_LIST,NULL);
					}
				} else {
					rest_server_error(result,&ok);
				}
			} else {
				/* Informacion sobre un usuario */
				data = malloc(sizeof(T_dictionary));
				dictionary_init(data);
				dictionary_add(data,"userid",userid);
				task_init(task,&token,T_USER_SHOW,data);
			}
		} else {
			printf("listado de usuarios\n");
			task_init(task,&token,T_USER_LIST,NULL);
		}

	/* PARA LOS TASK */
	} else if(0 == strcmp("task",value)) {
		/* Se solicita info de un task */
		isTaskStatus =1;
		free(task);
		parce_data((char *)url,'/',&pos,value);
		if(strlen(value)>0){
			printf("Nos solicitan cómo termino la tarea con id: %s\n",value);
			rest_server_get_task(&rest_server,(T_taskid *)value,&result,&size_result);
		} else {
			strcpy(result,"{task incorrecto}");
		}

	/* PARA CUALQUIER OTRA COSA */
	} else {
		rest_server_error(result,&ok);
	}

	if(ok){
		if(!isTaskStatus){
			sprintf(result,"{'task':'%s','status':'TODO'}",task_get_id(task));
			rest_server_add_task(&rest_server,task);
		}
	} else {
		printf("No OK : %s\n",result);
		task_destroy(&task);
	}
	send_page (connection, result);
	return ok;
}

static int iterate_post (void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
			const char *filename, const char *content_type,
			const char *transfer_encoding, const char *data, uint64_t off,
			size_t size){

	struct connection_info_struct *con_info = coninfo_cls;

	if(strlen(data)>0){
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

	if (0 == strcmp (method, "GET")){
		return handle_GET(connection,url);
	}
	if (0 == strcmp (method, "POST")){
		struct connection_info_struct *con_info = *con_cls;
		if (*upload_data_size != 0){
			MHD_post_process (con_info->postprocessor, upload_data,
					*upload_data_size);
			*upload_data_size = 0;
			return MHD_YES;
		} else {
			return handle_POST(connection,url,con_info);
		}
	}
	return send_page(connection, "TODO MAL");
}

void *rest_server_start(void *param){
	
	T_rest_server *r= (T_rest_server *)param;

	r-> rest_daemon = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION | MHD_USE_DEBUG,
			80, NULL, NULL, &answer_to_connection, NULL, MHD_OPTION_NOTIFY_COMPLETED,
			request_completed, NULL, MHD_OPTION_END);
}

void rest_server_init(T_rest_server *r, T_db *db){

	r->db = db;
	heap_task_init(&(r->tasks_todo));
	bag_task_init(&(r->tasks_done));
	if(0 != pthread_create(&(r->thread), NULL, &rest_server_start, r)){
		printf ("Imposible levantar el servidor REST\n");
		exit(2);
	}
	if(0 != pthread_create(&(r->do_task), NULL, &rest_server_do_task, r)){
		printf ("Imposible levantar el hilo para realizar tareas\n");
		exit(2);
	}
}

