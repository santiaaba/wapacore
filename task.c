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

void task_destroy(T_task **t){
	printf("Entramos a eliminar el task\n");
	if((*t)->data != NULL){
		printf("Eliminamos el diccionario\n");
		dictionary_destroy(&((*t)->data));
		//free((*t)->data);
	}
	printf("liberamos el resultado\n");
	free((*t)->result);
	printf("liberamos el task\n");
	free(*t);
}


char *task_get_result(T_task *t){
	return t->result;
}

char *task_get_id(T_task *t){
	return t->id;
}

T_tasktoken *tob_get_token(T_task *t){
	return t->token;
}

char *tob_get_id(T_task *t){
	return t->id;
}

void task_run(T_task *t, T_db *db){
	/* Ejecuta el JOB */
	printf("paso\n");
	t->status = T_RUNNING;
	printf("paso\n");

	switch(t->type){
		case T_GET_SITES: break;
		case T_GET_SITE: break;
		case T_GET_WORKERS: break;
		case T_GET_WORKER: break;
		case T_ADD_SITE: break;
		case T_DEL_SITE: break;
		case T_MOD_SITE: break;
		case T_STOP_WORKER: break;
		case T_START_WORKER: break;
		case T_STOP_SITE: break;
		case T_START_SITE: break;
	}
	t->status = T_DONE;
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

int heap_task_exist(T_heap_task *h, T_taskid id){
	/* indica si el trabajo existe en la cola */
	heap_t_node *aux;
	int exist=0;
	
	aux = h->first;
	while(!exist && aux!= NULL){
		exist = (strcmp(task_get_id(aux->data),id) == 0);
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

