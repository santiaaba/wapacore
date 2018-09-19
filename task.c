#include "task.h"

void random_task_id(T_taskid value){
	/*Genera un string random para task_id */
	char *string = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	int i,j;
	
	for(j=0;j<TASKID_SIZE;j++){
		i = rand() % 62;
		//printf("i=%i\n",i);
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

void random_dir(char *dir){
	/* Genera un dir y sub dir de dos digitos cada uno */
	char *string = "0123456789";
	int i,j;

	for(j=0;j<5;j++){
		if(j==2){
			dir[j]='/';
		} else {
			i = rand() % 10;
			dir[j] = string[i];
		}
	}
	dir[5]='\0';
}

/*****************************
	     TASK 
******************************/
void task_init(T_task *t, T_tasktoken *token, T_task_type type, T_dictionary *data){
	random_task_id(t->id);
	t->token = token;
	t->type = type;
	t->data = data;
	t->result = (char *)malloc(TASKRESULT_SIZE);
	t->result_size = TASKRESULT_SIZE;
	t->cloud = NULL;
	strcpy(t->result,"");
}

char *task_get_id(T_task *t){
	return t->id;
}

void task_destroy(T_task **t){
	printf("Entramos a eliminar el task\n");
	if((*t)->data != NULL){
		printf("Eliminamos el diccionario\n");
		dictionary_destroy(&((*t)->data));
	}
	printf("liberamos el resultado\n");
	free((*t)->result);
	printf("liberamos el task\n");
	free(*t);
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

char *tob_get_id(T_task *t){
	return t->id;
}

int task_user_list(T_task *t, T_db *db){
	MYSQL_RES *result;

	db_user_list(db,&result);
	json_user_list(&(t->result),&(t->result_size),result);
	t->status = T_DONE_OK;
}

int task_user_show(T_task *t, T_db *db){
	MYSQL_RES *result;
	char *id;

	printf("TASK: show user\n");
	id = dictionary_get(t->data,"userid");
	db_user_show(db,&result,id);
	if(json_user_show(&(t->result),&(t->result_size),result)){
		t->status = T_DONE_OK;
	} else {
		t->status = T_DONE_ERROR;
	}
}

int task_user_add(T_task *t, T_db *db){
	char result[100];

	if(db_user_add(db,t->data,result)){
		strcpy(t->result,"usuario agregado correctamente");
		t->status = T_DONE_OK;
	} else {
		strcpy(t->result,result);
		t->status = T_DONE_ERROR;
	}
}

int task_user_mod(T_task *t, T_db *db){
	char result[100];
	printf("Task modificar usuario\n");
	if(db_user_mod(db,t->data,result)){
		strcpy(t->result,"usuario modificado");
		t->status = T_DONE_OK;
	} else {
		printf("result: %s\n",result);
		strcpy(t->result,result);
		t->status = T_DONE_ERROR;
	}
}

int task_susc_add(T_task *t, T_db *db){
	char result[100];
	if(db_susc_add(db,t->data,result)){
		strcpy(t->result,"suscripcion generada");
		t->status = T_DONE_OK;
	} else {
		strcpy(t->result,result);
		t->status = T_DONE_ERROR;
	}

}

int task_susc_del(T_task *t, T_db *db){
	char result[100];
	if(db_susc_del(db,t->data,result)){
		strcpy(t->result,"suscripcion eliminada");
		t->status = T_DONE_OK;
	} else {
		strcpy(t->result,result);
		t->status = T_DONE_ERROR;
	}
}

int task_user_del(T_task *t, T_db *db){
	char result[100];
	if(db_user_del(db,t->data,result)){
		strcpy(t->result,"suscripcion eliminada");
		t->status = T_DONE_OK;
	} else {
		strcpy(t->result,result);
		t->status = T_DONE_ERROR;
	}
}

int task_susc_mod(T_task *t, T_db *db){

	char result[100];
	printf("Task modificar suscripcion\n");
	if(db_susc_mod(db,t->data,result)){
		strcpy(t->result,"suscripcion modificada");
		t->status = T_DONE_OK;
	} else {
		printf("result: %s\n",result);
		strcpy(t->result,result);
		t->status = T_DONE_ERROR;
	}
}

int task_susc_show(T_task *t, T_db *db){
	MYSQL_RES *result;
	char *susc_id = dictionary_get(t->data,"susc_id");

	/* Recolectamos los datos de la suscripcion global */
	db_susc_show(db,susc_id,&result);
	if(!json_susc_show(&(t->result),&(t->result_size),result)){	
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
        json_susc_list(&(t->result),&(t->result_size),result);
        t->status = T_DONE_OK;
}

void task_json_result(T_task *t, char **result, int *result_size){

	char aux[20];

	task_print_status(t, aux);
	*result=(char *)realloc(*result,(t->result_size) + 200);
	*result_size = t->result_size + 200;
        sprintf(*result,"{\"taskid\":\"%s\",\"status\":\"%s\",\"data\":%s}",t->id,aux,t->result);
}


int task_site_list(T_task *t){
	/* Lista los sitios de una suscripcion en particular */
	char send_message[100];
	char *rcv_message = NULL;
	char *json_data;
	int json_data_size;
	uint32_t rcv_message_size = 0;
	int pos;
	char value[200];

	if(t->status == T_TODO){
		/* Es la primera vez que tratamos esta tarea */
		sprintf(send_message,"lsusc_id|%s",dictionary_get(t->data,"susc_id"));
		// pasamos el tamano del mensaje +1 para incluir el '\0'
		if(cloud_send_receive(t->cloud,send_message,strlen(send_message)+1,&rcv_message,&rcv_message_size)){
			pos=1;
			parce_data(rcv_message,'|',&pos,value);
			printf("El task_id de la nube es: -%s-\n",value);
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
	} else if(t->status == T_WAITING){
		/* Solicitamos a la nube estado del task */
		sprintf(send_message,"t%s\0",dictionary_get(t->data,"c_task_id"));
		if(cloud_send_receive(t->cloud,send_message,strlen(send_message),&rcv_message,&rcv_message_size)){
			printf("Resultado obtenido por la nube: %c\n",rcv_message[0]);
			if(rcv_message[0] == '0'){
				/* El task no ha terminado del lado de la nube */
			} else if(rcv_message[0] == '1'){
				t->result=(char *)realloc(t->result,rcv_message_size);
				memcpy(t->result,&rcv_message[1],rcv_message_size-1);
				t->result_size = rcv_message_size;
				t->status = T_DONE_OK;
			} else {
				/* task no existe en la nube */
				t->status = T_DONE_ERROR;
			}
		}
	}
	task_print_status(t,value);
	free(rcv_message);
}

int task_site_show(T_task *t){
	printf("IMPLEMENTAR\n");
}
int task_site_add(T_task *t){
	printf("IMPLEMENTAR\n");
}
int task_site_del(T_task *t){
	printf("IMPLEMENTAR\n");
}
int task_site_mod(T_task *t){
	printf("IMPLEMENTAR\n");
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
			case T_SUSC_ADD: task_susc_add(t,db); break;
			case T_SUSC_MOD: task_susc_mod(t,db); break;
		}
	} else {
		printf("Acciones sobre una nube\n");
		/* Son acciones sobre alguna nube */
		if(t->cloud == NULL){
			/* Averiguamos la nube */
			if( t->type <= T_SITE_DEL){
				/* Acciones sobre una nube web */
				valor = dictionary_get(t->data,"susc_id");
				if(! db_get_cloud_id(db,atoi(valor),C_WEB,&cloud_id)){
					printf("ERROR. Cloud no encontrada\n");
				}
				c = list_cloud_find_id(cl,cloud_id);
				t->cloud = c;
			}
		}

		switch(t->type){
			/* SITES */
			case T_SITE_LIST: task_site_list(t); break;
			case T_SITE_SHOW: task_site_show(t); break;
			case T_SITE_ADD: task_site_add(t); break;
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

	printf("PRINT BAG\n");
	aux = h->first;
	while(aux!= NULL){
		printf("Job_ID: %s\n",task_get_id(aux->data));
		aux = aux->next;
	}
	printf("END PRINT BAG\n");
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
		//printf("Comparamos -%s- con -%s-\n",id,task_get_id(b->actual->data));
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

	printf("PRINT BAG\n");
	aux = b->first;
	while(aux!= NULL){
		printf("Job_ID: %s\n",task_get_id(aux->data));
		aux = aux->next;
	}
	printf("END PRINT BAG\n");
}

