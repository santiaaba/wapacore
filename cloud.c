#include "cloud.h"

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
	/* funcion de uso interno */
	/* Retorna 1 si cambio el estado. 0 en caso contrario */
	char aux[100];
	char aux2[100];

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

int cloud_send_receive(T_cloud *cloud, char *send_message, int send_message_size,
			char **rcv_message, int *rcv_size){
	/* Envia una instruccion que puede superar una transmision y recebe la
 	 * respuesta a la misma */

	char buffer[BUFFER_SIZE];
	int pos;
	char *p;
	char fin;
	int c;

	/* El envio puede que conlleve varias transmisiones */
	p = send_message;
	c = 0;

	//printf("cloud_send_receive- ENTROOO\n");
	/* Enviar */
	printf("Enviamos a la Nube: %i:%i - %s\n",BUFFER_SIZE,send_message_size,send_message);
        send_message_size ++;         //contabilizamos el '\0' del final del string
	while(c < send_message_size){
		/* Hay que incluir un header de tamano HEADER_SIZE */
		if((send_message_size - c + HEAD_SIZE) < BUFFER_SIZE){
			printf("Entra en una transmision %i \n",send_message_size - c - HEAD_SIZE);
			memcpy(buffer + HEAD_SIZE,p,send_message_size - c);
			buffer[0] = '0';
			c += send_message_size - c;
		} else{
			printf("Va a requerir una transmision mas\n");
			memcpy(buffer + HEAD_SIZE,p,BUFFER_SIZE - HEAD_SIZE);
			buffer[0] = '1';
			printf("Va a requerir una transmision mas: %s\n",buffer);
			c += BUFFER_SIZE - HEAD_SIZE;
			p += c;
		}
		printf("cloud_send_receive - SEND:-%s-\n",buffer);
		if(!send(cloud->socket,buffer,BUFFER_SIZE,0)){
			cloud_change_status(cloud,C_UNKNOWN);
			cloud_connect(cloud);
			return 0;
		}
		/* Solo nos quedamos esperando confirmacion de mas datos
 		 * si emos enviado un 1 en el encabezado */
		if(buffer[0] == '1'){
			if(recv(cloud->socket,buffer,BUFFER_SIZE,0)<0){
				cloud_change_status(cloud,C_UNKNOWN);
				cloud_connect(cloud);
				return 0;
			}
			printf("cloud_send_receive - Resive:-%s-\n",buffer);
		}
	}
	//printf("Recibimos respuesta\n");
	//sleep(10);
	/* Recibir */
	fin = '1';
	printf("cloud_send_receive: Ahora recibimos\n");
	*rcv_size=-1;
	while(fin == '1'){
		if(recv(cloud->socket,buffer,BUFFER_SIZE,0)<0){
			cloud_change_status(cloud,C_UNKNOWN);
			cloud_connect(cloud);
			return 0;
		}
		printf("send_recive - recibido: %s\n",buffer);
		pos = *rcv_size + 1;
		*rcv_size += BUFFER_SIZE - HEAD_SIZE;
		*rcv_message=(char *)realloc(*rcv_message,*rcv_size);
		printf("Pasamos realloc\n");
		memcpy(*rcv_message+pos,&(buffer[HEAD_SIZE]),BUFFER_SIZE - HEAD_SIZE);
		printf("Pasamos memcpy\n");
		fin = buffer[0];
		/* Solo enviamos confirmacion de resepcion de mas datos
 		 * si hemos recibido un 1 en el encabezado */
		if(fin == '1'){
			printf("FIN ES 1!!!!!\n");
			send(cloud->socket,"1\0", BUFFER_SIZE,0);
		}
	}
	printf("Mensaje completo: %s\n",*rcv_message);
	//sleep(10);
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
