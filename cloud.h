#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifndef CLOUD_H
#define CLOUD_H

#define BUFFER_SIZE	1024
#define HEAD_SIZE	1

typedef enum { C_WEB, C_MSSQMLDB, C_MYSQLDB} T_cloud_type;
typedef enum { C_ONLINE, C_OFFLINE, C_PREPARED, C_BROKEN, C_UNKNOWN} T_cloud_status;

/************************
 *      CLOUD	        *
 ************************/

typedef struct {
	unsigned int id;   //4 bytes
	T_cloud_type type;
	char name[100];
	char ipv4[15];
	char port[15];
	char user[50];
	char pass[50];
	unsigned long time_change_status;
	int is_changed;
	T_cloud_status status;
	T_cloud_status last_status;
	struct sockaddr_in server;
	int socket;
} T_cloud;

void cloud_init(T_cloud *c, unsigned int id, T_cloud_type type, char *name, char *ipv4, char *user, char *pass, char *port);
unsigned int cloud_get_id(T_cloud *c);
char *cloud_get_name(T_cloud *c);
char *cloud_get_ipv4(T_cloud *c);
char *cloud_get_user(T_cloud *c);
char *cloud_get_pass(T_cloud *c);
int cloud_connect(T_cloud *c);
int cloud_end_connect(T_cloud *c);
int cloud_send_receive(T_cloud *c, char *send_message, int send_message_size, char **rcv_message, int *rcv_size);

/************************
 *      LIST CLOUD      *
 ************************/

typedef struct list_cloud T_list_cloud;
typedef struct c_node {
        T_cloud *data;
        struct c_node *next;
} list_c_node;

struct list_cloud{
        unsigned int size;
        list_c_node *first;
        list_c_node *last;
        list_c_node *actual;
};

/* Inicializa la estructura de lista */
void list_cloud_init(T_list_cloud *l);

/* Agrega un elemento al final de la lista */
void list_cloud_add(T_list_cloud *l, T_cloud *c);

/* Retorna el elemento actualmente apuntado en la lista */
T_cloud *list_cloud_get(T_list_cloud *l);

/* Coloca el punto al inicio de la lista*/
void list_cloud_first(T_list_cloud *l);

/* Avanza el puntero un elemento en la lista*/
void list_cloud_next(T_list_cloud *l);

/* Retorna la cantidad de elementos en la lista */
unsigned int list_cloud_size(T_list_cloud *l);

/* Indica si el puntero esta al finald e la lista */
int list_cloud_eol(T_list_cloud *l);

/* retorna el elemento solicitado por su id. NULL si no existe*/
T_cloud *list_cloud_find_id(T_list_cloud *l, int cloud_id);

#endif
