#include "cloud.h"

void int_to_4bytes(uint32_t *i, char *_4bytes){
        memcpy(_4bytes,i,4);
}

void _4bytes_to_int(char *_4bytes, uint32_t *i){
        memcpy(i,_4bytes,4);
}

/************************
 * 	CLOUD		*
 ************************/

void cloud_init(T_cloud *c, unsigned int id, T_cloud_type type, char *name, char *ipv4, char *user, char *pass, char *port){

	c->id=id;
	c->type = type;
	strcpy(c->name,name);
	strcpy(c->ipv4,ipv4);
	strcpy(c->port,port);
	strcpy(c->user,user);
	strcpy(c->pass,pass);
	c->status = C_ONLINE;
	c->last_status = C_ONLINE;
	c->time_change_status = time(0);
	c->is_changed = 0;
	c->server.sin_addr.s_addr = inet_addr(ipv4);
	c->server.sin_family = AF_INET;
	c->server.sin_port = htons(atoi(port));
	cloud_connect(c);
}

unsigned int cloud_get_id(T_cloud *c){
	return c->id;
}
char *cloud_get_name(T_cloud *c){
	return c->name;
}
char *cloud_get_ipv4(T_cloud *c){
	return c->ipv4;
}
char *cloud_get_user(T_cloud *c){
	return c->user;
}
char *cloud_get_pass(T_cloud *c){
	return c->pass;
}

void cloud_change_status(T_cloud *c, T_cloud_status s){
	/* Solo actualizamos el estado si es distinto
	* del que ya posee */
	if(c->status != s){
		c->time_change_status = (unsigned long)time(0);
		c->last_status = c->status;
		c->status = s;
		c->is_changed = 1;
	}
}


int cloud_connect(T_cloud *c){
	close(c->socket);
	c->socket = socket(AF_INET , SOCK_STREAM , 0);
	if (connect(c->socket , (struct sockaddr *) &(c->server) , sizeof(c->server)) < 0){
		printf("Controller %s NO CONECTA\n",c->name);
		return 0;
	}
	printf("Controller %s CONECTO\n",c->name);
	return 1;
}

int cloud_end_connect(T_cloud *c){
	close(c->socket);
}

T_cloud_type cloud_get_type(T_cloud *c){
	return c->type;
}

T_cloud_status cloud_get_status(T_cloud *c){
	return c->status;
}

int cloud_check(T_cloud *c){
	char *rcv_message = NULL;
	uint32_t rcv_message_size = 0;

	if(cloud_send_receive(c,"c\0",2,&rcv_message,&rcv_message_size)){
		if(rcv_message[0] != 1){
			if(c->status == C_BROKEN || c->status == C_UNKNOWN){
				cloud_change_status(c,C_PREPARED);
			} else {
				if((c->status == C_PREPARED) &&
				((unsigned long)time(0) - c->time_change_status) > TIMEONLINE){
					/* Responde, pasa el chequeo,
 					   ya estaba en PREPARED y paso el tiempo */
					cloud_change_status(c,C_ONLINE);
				}
			
			}
		} else {
			printf("cloud %s ROTO!\n",cloud_get_name(c));
			cloud_change_status(c,C_BROKEN);
		}
	} else {
		printf("cloud %s NO RESPONDE!\n",cloud_get_name(c));
	}
	if (c->is_changed){
		c->is_changed = 0;
		return 1;
	} else {
		return 0;
	}
}

int cloud_send_receive(T_cloud *cloud, char *send_message, uint32_t send_message_size,
			char **rcv_message, uint32_t *rcv_message_size){

	char buffer[BUFFER_SIZE];
	char printB[BUFFER_SIZE+1];
	uint32_t parce_size;
	int first_message=1;
	int pos;
	uint32_t c=0;	//cantidad de datos enviados o recibidos

	if( send_message[send_message_size-1] != '\0'){
		printf("cloud_send_receive: ERROR. send_message no termina en \\0\n");
		return 0;
	}

	printf("SEND-------SEND----SEND-----\n");
	printf("Mensaje a la Nube. send_message_size=:%i, send_message=%s\n",send_message_size,send_message);
	/* Los 4 primeros bytes del header es el tamano total del mensaje */
	int_to_4bytes(&send_message_size,buffer);

	while(c < send_message_size){
		/* Hay que incluir un header de tamano HEADER_SIZE */
		if(send_message_size - c + HEADER_SIZE < BUFFER_SIZE){
			/* Entra todo en el buffer */
			parce_size = send_message_size - c ;
		} else {
			/* No entra todo en el buffer */
			parce_size = BUFFER_SIZE - HEADER_SIZE;
		}
		int_to_4bytes(&parce_size,&(buffer[4]));
		memcpy(buffer + HEADER_SIZE,send_message + c,parce_size);
		c += parce_size;
		printf("Listos para enviar %i\n",cloud->socket);
		if(send(cloud->socket,buffer,BUFFER_SIZE,0)<0){
			printf("Socket no responde\n");
			cloud_change_status(cloud,C_UNKNOWN);
			cloud_connect(cloud);
			return 0;
		}
	}

	/* Recibir */
	c=0;
	/* Al menos una recepcion esperamos recibir */
	printf("RECEIV-------RECEIV-------RECEIV-----\n");
	int_to_4bytes(&c,buffer);
	int_to_4bytes(&c,&(buffer[4]));
	do{
		if(recv(cloud->socket,buffer,BUFFER_SIZE,0)<0){
			cloud_change_status(cloud,C_UNKNOWN);
			cloud_connect(cloud);
			return 0;
		}
		/* Del header obtenemos el tamano de los datos que
 		 * recibiremos */
		if(first_message){
			first_message=0;
			_4bytes_to_int(buffer,rcv_message_size);
			*rcv_message=(char *)realloc(*rcv_message,*rcv_message_size);
		}

		_4bytes_to_int(&(buffer[4]),&parce_size);
		memcpy(*rcv_message+c,&(buffer[HEADER_SIZE]),parce_size);
		c += parce_size;
	} while (c < *rcv_message_size);
	printf("RECEIV completo: %s\n",*rcv_message);
	return 1;
}

/************************
 * 	LIST CLOUD	*
 ************************/

/* Inicializa la estructura de lista */
void list_cloud_init(T_list_cloud *l){
	l->first = NULL;
	l->actual = NULL;
	l->last = NULL;
	l->size = 0;
}

/* Agrega un elemento al final de la lista */
void list_cloud_add(T_list_cloud *l, T_cloud *c){
	list_c_node *new;
	list_c_node *aux;

	new = (list_c_node*)malloc(sizeof(list_c_node));
	new->next = NULL;
	new->data = c;
	l->size++;

	if(l->first == NULL){
		l->first = new;
		l->last = new;
	} else {
		l->last->next = new;
		l->last = new;
	}
}

/* Retorna el elemento actualmente apuntado en la lista */
T_cloud *list_cloud_get(T_list_cloud *l){
	if(l->actual!=NULL)
		return l->actual->data;
	else
		return NULL;
}

/* Coloca el punto al inicio de la lista*/
void list_cloud_first(T_list_cloud *l){
	l->actual = l->first;
}

/* Avanza el puntero un elemento en la lista*/
void list_cloud_next(T_list_cloud *l){
	if(l->actual != NULL){
		l->actual = l->actual->next;
	}
}

/* Retorna la cantidad de elementos en la lista */
unsigned int list_cloud_size(T_list_cloud *l){
	return l->size;
}

/* Indica si el puntero esta al finald e la lista */
int list_cloud_eol(T_list_cloud *l){
	return (l->actual == NULL);
}

/* retorna el elemento solicitado por su id. NULL si no existe*/
T_cloud *list_cloud_find_id(T_list_cloud *l, int cloud_id){
	list_c_node *aux;
	int exist = 0;

	aux = l->first;
	while(aux != NULL && !exist){
		exist = (cloud_get_id(aux->data) == cloud_id);
		if(!exist){ aux = aux->next;}
	}
	if(exist){
		return aux->data;
	} else {
		return NULL;
	}
}
