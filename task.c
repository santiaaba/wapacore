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
void task_init(T_task *t, T_tasktoken *token, T_task_type type, T_dictionary *data){
	random_task_id(t->id);
	t->token = token;
	t->type = type;
	t->data = data;
	t->step=0;
	t->status = T_TODO;
	t->result = (char *)malloc(TASKRESULT_SIZE);
	//t->result_size = TASKRESULT_SIZE;
	t->cloud = NULL;
	strcpy(t->result,"");
}

char *task_get_id(T_task *t){
	return t->id;
}

void task_destroy(T_task **t){
	if((*t)->data != NULL){
		dictionary_destroy(&((*t)->data));
	}
	//printf("TASK_DESTROY-dic\n");
	free((*t)->result);
	//printf("TASK_DESTROY-result\n");
	free(*t);
	//printf("TASK_DESTROY-fin\n");
}

T_task_status task_get_status(T_task *t){
	return t->status;
}

T_tasktoken *task_get_token(T_task *t){
	return t->token;
}

void task_print_status(T_task *t, char *s){
	switch(t->status){
	case T_WAITING: strcpy(s,"waiting"); break;
	case T_TODO: strcpy(s,"todo"); break;
	case T_DONE_OK: strcpy(s,"done_ok"); break;
	case T_DONE_ERROR: strcpy(s,"done_error"); break;
	}
}

void task_set_result(T_task *t, T_task_status status, char *message){
	//printf("TASK_SET_RESULT\n");
	//printf("%s\n",message);
	t->status = status;
	//t->result_size = strlen(message);
	t->result=(char *)realloc(t->result,strlen(message)+1);
	strcpy(t->result,message);
	//printf("TASK_SET_RESULT-fin\n");
}

char *tob_get_id(T_task *t){
	return t->id;
}

void task_user_list(T_task *t, T_db *db){
	MYSQL_RES *result;
	char *message=NULL;
	char *aux=NULL;

	if(!db_user_list(db,&result)){
		task_set_result(t,T_DONE_ERROR,"{\"code\":\"300\",\"info\":\"ERROR FATAL\"}");
	} else {
		json_user_list(&aux,result);
		message=(char *)malloc(strlen(aux)+ 25);
		sprintf(message,"{\"code\":\"200\",\"info\":%s",aux);
		task_set_result(t,T_DONE_OK,message);
	}
	free(message);
	free(aux);
}

void task_user_show(T_task *t, T_db *db){
	MYSQL_RES *result;
	char *aux=NULL;
	char *message=NULL;
	char *id;

	printf("TASK_USER_SHOW\n");
	id = dictionary_get(t->data,"user_id");
	if(!db_user_show(db,&result,id)){
		task_set_result(t,T_DONE_ERROR,"{\"code\":\"300\",\"info\":\"ERROR FATAL\"}");
	} else {
		if(!json_user_show(&aux,result)){
			task_set_result(t,T_DONE_ERROR,"{\"code\":\"320\",\"info\":\"Usuario inexistente\"}");
		}
	}
	message=(char *)malloc(strlen(aux)+ 25);
	sprintf(message,"{\"code\":\"200\",\"info\":%s",aux);
	task_set_result(t,T_DONE_OK,message);
	free(message);
	free(aux);
}

void task_user_add(T_task *t, T_db *db){
	char result[200];

	if(db_user_add(db,t->data,result)){
		task_set_result(t,T_DONE_OK,result);
	} else {
		task_set_result(t,T_DONE_ERROR,result);
	}
}


int task_user_mod(T_task *t, T_db *db){
	char result[100];

	printf("TASK_USER_MOD\n");
	if(db_user_mod(db,t->data,result)){
		 task_set_result(t,T_DONE_OK,result);
	} else {
		task_set_result(t,T_DONE_ERROR,result);
	}
}

/*-------------------------- ACA QUEDAMOS ---------------*/

int task_susc_add(T_task *t, T_db *db, T_list_cloud *cl){
	/* Agrega una suscripcion. Se conecta a las
 	 * nubes que correspondan para dar de alta
 	 * los elementos de la suscripcion */

	char result[100];
	char *susc_id;
	int cloud_id;
	char send_message[100];
	T_cloud *c;

	/* Damos de alta la suscripcion en la base de datos del Core
 	   si corresponde */
	if(t->status == T_TODO){
		switch (t->step){
			case 0:	// Creamos la suscripcion en la base de datos
				db_susc_add(db,t->data,result);
				break;
			case 1:	// Creamos la suscripcion en la nube web
				susc_id = dictionary_get(t->data,"susc_id");
				if(db_get_cloud_id(db,atoi(susc_id),C_WEB,&cloud_id)){
					c = list_cloud_find_id(cl,cloud_id);
					t->cloud = c;
				}
				break;
			case 2:	// Creamos la suscripcion en la nube web
				susc_id = dictionary_get(t->data,"susc_id");
				if(db_get_cloud_id(db,atoi(susc_id),C_MYSQLDB,&cloud_id)){
					c = list_cloud_find_id(cl,cloud_id);
					t->cloud = c;
				} else {
					t->cloud = NULL;
				}
				break;
			case 3:	// Creamos la suscripcion en la nube web
				susc_id = dictionary_get(t->data,"susc_id");
				if(db_get_cloud_id(db,atoi(susc_id),C_MSSQLDB,&cloud_id)){
					c = list_cloud_find_id(cl,cloud_id);
					t->cloud = c;
				} else {
					t->cloud = NULL;
				}
				break;
		}
		if(t->step > 0 && t->cloud != NULL){
			sprintf(send_message,"0susc_id|%s", susc_id);
			task_cloud_send(t,send_message);	//Esto me coloca el estado en T_WAITING
		}
		t->step ++;
	} else if(t->status == T_WAITING){
		task_cloud_get(t);
		if(t->result_code == 200){
			t->status = T_TODO;
		}
	}
}

int task_susc_del(T_task *t, T_db *db, T_list_cloud *cl){
	char result[100];
	char *susc_id;
	int cloud_id;
	char send_message[100];
	T_cloud *c;

	/* Verificamos que ese suscripción exista para el
 	 * usuario indicado */

	susc_id = dictionary_get(t->data,"susc_id");
	if(t->status == T_TODO){
		switch (t->step){
			case 0:	// Borramos la suscripcion en la nube web
				if(db_get_cloud_id(db,atoi(susc_id),C_WEB,&cloud_id)){
					c = list_cloud_find_id(cl,cloud_id);
					t->cloud = c;
				}
				break;
			case 1:	// Borramos la suscripcion en la nube web
				if(db_get_cloud_id(db,atoi(susc_id),C_MYSQLDB,&cloud_id)){
					c = list_cloud_find_id(cl,cloud_id);
					t->cloud = c;
				} else {
					t->cloud = NULL;
				}
				break;
			case 2:	// Borramos la suscripcion en la nube web
				if(db_get_cloud_id(db,atoi(susc_id),C_MSSQLDB,&cloud_id)){
					c = list_cloud_find_id(cl,cloud_id);
					t->cloud = c;
				} else {
					t->cloud = NULL;
				}
				break;
			case 3:	// Borramos definitivamente de la base de datos
				if(db_susc_del(db,t->data,result)){
					strcpy(t->result,"suscipcion modificada");
					t->status = T_DONE_OK;
				} else {
					strcpy(t->result,result);
					t->status = T_DONE_ERROR;
				}
		}
		if(t->step < 3 && t->cloud != NULL){
			sprintf(send_message,"1susc_id|%s", susc_id);
			task_cloud_send(t,send_message);	//Esto me coloca el estado en T_WAITING
		}
		t->step ++;
	} else if(t->status == T_WAITING){
		task_cloud_get(t);
		if(t->result_code == 200){
			t->status = T_TODO;
		}
	}
}

int task_user_del(T_task *t, T_db *db){
	char result[100];
	if(db_user_del(db,t->data,result)){
		strcpy(t->result,"usuario eliminada");
		t->status = T_DONE_OK;
	} else {
		strcpy(t->result,result);
		t->status = T_DONE_ERROR;
	}
}

int task_susc_mod(T_task *t, T_db *db){

	char result[100];
	if(db_susc_mod(db,t->data,result)){
		strcpy(t->result,"suscripcion modificada");
		t->status = T_DONE_OK;
	} else {
		strcpy(t->result,result);
		t->status = T_DONE_ERROR;
	}
}

int task_susc_show(T_task *t, T_db *db){
	MYSQL_RES *result;
	char *susc_id = dictionary_get(t->data,"susc_id");

	/* Recolectamos los datos de la suscripcion global */
	db_susc_show(db,susc_id,&result);
	if(!json_susc_show(&(t->result),result)){	
		t->status = T_DONE_ERROR;
		return 0;
	}
	t->status = T_DONE_OK;
	return 1;
}

int task_susc_list(T_task *t, T_db *db){
	MYSQL_RES *result;
	char *user_id = dictionary_get(t->data,"user_id");

	db_susc_list(db,user_id,&result);
	json_susc_list(&(t->result),result);
	t->status = T_DONE_OK;
}

void task_json_result(T_task *t, char **result){

	char aux[20];

	task_print_status(t, aux);
	*result=(char *)realloc(*result,strlen(t->result) + 200);
	sprintf(*result,"{\"taskid\":\"%s\",\"status\":\"%s\",\"data\":%s}",t->id,aux,t->result);
}

int task_cloud_send(T_task *t, char *send_message){
	/* Envia una solicitud a la nube. Se guarda el
 	 * task id en la estructura del task. */

	char *rcv_message = NULL;
	uint32_t rcv_message_size = 0;
	char value[100];
	int pos=1;

	// pasamos el tamano del mensaje +1 para incluir el '\0'
	if(cloud_send_receive(t->cloud,send_message,strlen(send_message)+1,&rcv_message,&rcv_message_size)){
		parce_data(rcv_message,'|',&pos,value);
		if(rcv_message[0] == '1'){
			dictionary_add(t->data,"c_task_id",value);
			t->status = T_WAITING;
		} else {
			dictionary_add(t->data,"error_message",value);
			t->status = T_DONE_ERROR;
		}
	} else {
		t->status = T_DONE_ERROR;
	}
}

int task_cloud_get(T_task *t){
	/* Solicita a la nube el estado de un task
 	 * y el resultado del mismo */
	char send_message[100];
	char *rcv_message = NULL;
	char result_code[10];
	int pos = 1;
	uint32_t rcv_message_size = 0;

	sprintf(send_message,"t%s",dictionary_get(t->data,"c_task_id"));
	if(cloud_send_receive(t->cloud,send_message,strlen(send_message)+1,&rcv_message,&rcv_message_size)){
		if(rcv_message[0] == '0'){
			/* El task no ha terminado del lado de la nube */
		} else if(rcv_message[0] == '1'){
			parce_data(rcv_message + 1,'|',&pos,result_code);
			t->result_code = atoi(result_code);

			t->result=(char *)realloc(t->result,rcv_message_size - pos);
			memcpy(t->result,&rcv_message[pos],rcv_message_size - pos);
			//t->result_size = rcv_message_size - pos;
			t->status = T_DONE_OK;
		} else {
			/* task no existe en la nube */
			t->status = T_DONE_ERROR;
		}
	}
	free(rcv_message);
}

int task_site_list(T_task *t){
	/* Solicita al Controller el listado de sitios de una
 	 * suscripcion. El Controller ya lo retorna en formato json */
	char send_message[100];

	if(t->status == T_TODO){
		sprintf(send_message,"lsusc_id|%s",dictionary_get(t->data,"susc_id"));
		task_cloud_send(t,send_message);
	} else if(t->status == T_WAITING){
		task_cloud_get(t);
	}
}

int task_site_show(T_task *t, T_db *db){
	/* Solicita al Controller los datos de un sitio.
 	 El Controller ya lo retorna en formato json */
	char send_message[100];

	/* Verificamos que el susc_id sea del cliente user_id */

	if(db_user_susc(db,dictionary_get(t->data,"user_id"),
			dictionary_get(t->data,"susc_id"))){
		if(t->status == T_TODO){
			sprintf(send_message,"ssite_id|%s|susc_id|%s",
				dictionary_get(t->data,"site_id"),
				dictionary_get(t->data,"susc_id"));
			task_cloud_send(t,send_message);
		} else if(t->status == T_WAITING){
			task_cloud_get(t);
		}
	} else {
	}
}

int task_site_add(T_task *t, T_db *db){
	/* Solicita a la nube web agregar un sitio nuevo.
 	 * la nube ya retorna en formato json el resultado */
	char send_message[100];
	char *susc_id;

	if(t->status == T_TODO){
		/* Realizamos cantidad de sitios permitidos*/
		susc_id = dictionary_get(t->data,"susc_id");
		if(!db_accept_add_site(db,susc_id)){
			task_set_result(t,T_DONE_ERROR,"{\"code\":\"303\",\"info\":\"ERROR FATAL\"}");
		} else {
			/* Enviamos la accion a la nube */
			sprintf(send_message,"aname|%s|susc_id|%s",
				susc_id,dictionary_get(t->data,"susc_id"));
			task_cloud_send(t,send_message);
		}

	} else if(t->status == T_WAITING){
		task_cloud_get(t);
	}
}

int task_site_del(T_task *t){
	char send_message[100];
	if(t->status == T_TODO){
		sprintf(send_message,"dsusc_id|%s|site_id|%s",
			dictionary_get(t->data,"susc_id"),
			dictionary_get(t->data,"site_id")
		);
		task_cloud_send(t,send_message);
	} else if(t->status == T_WAITING){
		task_cloud_get(t);
	}
}

int task_site_mod(T_task *t){
}

void task_run(T_task *t, T_db *db, T_list_cloud *cl){
	T_cloud *c;
	char *valor;
	int cloud_id;

	if(t->type <= T_SUSC_DEL ){
		/* No son acciones sobre una nube */
		switch(t->type){
			/* USERS */
			case T_USER_LIST: task_user_list(t,db); break;
			case T_USER_SHOW: task_user_show(t,db); break;
			case T_USER_ADD: task_user_add(t,db); break;
			case T_USER_MOD: task_user_mod(t,db); break;
			case T_USER_DEL: task_user_del(t,db); break;
	
			/* SUSCRIPTION */
			case T_SUSC_LIST: task_susc_list(t,db); break;
			case T_SUSC_SHOW: task_susc_show(t,db); break;
			case T_SUSC_ADD: task_susc_add(t,db,cl); break;
			case T_SUSC_MOD: task_susc_mod(t,db); break;
			case T_SUSC_DEL: task_susc_del(t,db,cl); break;
			default:
				t->status = T_DONE_ERROR;
		}
	} else {
		/* Son acciones sobre alguna nube */
		if(t->cloud == NULL){
			/* Averiguamos la nube */
			if( t->type <= T_SITE_DEL){
				/* Acciones sobre una nube web */
				valor = dictionary_get(t->data,"susc_id");
				if(! db_get_cloud_id(db,atoi(valor),C_WEB,&cloud_id)){
					t->status = T_DONE_ERROR;
					return;
				} else {
					c = list_cloud_find_id(cl,cloud_id);
					t->cloud = c;
				}
			}
		}

		switch(t->type){
			/* SITES */
			case T_SITE_LIST: task_site_list(t); break;
			case T_SITE_SHOW: task_site_show(t,db); break;
			case T_SITE_ADD: task_site_add(t,db); break;
			case T_SITE_MOD: task_site_mod(t); break;
			case T_SITE_DEL: task_site_del(t); break;
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

