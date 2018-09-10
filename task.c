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

char *task_get_result(T_task *t){
	return t->result;
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
	case T_RUNNING: strcpy(s,"running"); break;
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
	char *name = dictionary_get(t->data,"name");
	char *pass = dictionary_get(t->data,"pass");
	char *email = dictionary_get(t->data,"email");
	char result[100];

	if(db_user_add(db,name,pass,email,result)){
		strcpy(t->result,"usuario agregado correctamente");
		t->status = T_DONE_OK;
	} else {
		strcpy(t->result,result);
		t->status = T_DONE_ERROR;
	}
}

int task_susc_add(T_task *t, T_db *db){
	char *user_id = dictionary_get(t->data,"user_id");
	char *plan_id = dictionary_get(t->data,"plan_id");

	char result[100];
	if(db_suscrip_add(db,user_id,plan_id,result)){
		strcpy(t->result,"suscripcion generada");
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
	if(!json_susc_show(&(t->result),&(t->result_size),result)){	
		t->status = T_DONE_ERROR;
		return 0;
	}

	/* Si corresponde, recolectamos los datos de la suscripcion web */
	db_susc_show_web(db,susc_id,&result);
	IMPLEMENTAR!!!

	/* Si corresponde, recolectamos los datos de la suscripcion MSsql */

	/* Si corresponde, recolectamos los datos de la suscripcion Mysql */

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

void task_run(T_task *t, T_db *db){
	/* Ejecuta el JOB */
	t->status = T_RUNNING;

	switch(t->type){
		/* USERS */
		case T_USER_LIST: task_user_list(t,db); break;
		case T_USER_SHOW: task_user_show(t,db); break;
		case T_USER_ADD: task_user_add(t,db); break;
		case T_USER_MOD: break;
		case T_USER_DEL: break;

		/* SUSCRIPTION */
		case T_SUSC_LIST: task_susc_list(t,db); break;
		case T_SUSC_SHOW: task_susc_show(t,db); break;
		case T_SUSC_ADD: task_susc_add(t,db); break;
		case T_SUSC_MOD: break;
		case T_SUSC_DEL: break;
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
		printf("Job_ID: %s\n",task_get_id(aux->data));
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

	printf("Entroooo\n");
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
		printf("Comparamos -%s- con -%s-\n",id,task_get_id(b->actual->data));
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

