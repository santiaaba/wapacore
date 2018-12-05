#include <microhttpd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "json.h"
#include "dictionary.h"
#include "config.h"
#include "task.h"
#include "valid.h"

#ifndef REST_SERVER_H
#define REST_SERVER_H

#define PORT		8888
#define POSTBUFFERSIZE  512
#define MAXNAMESIZE     20
#define MAXANSWERSIZE   512
#define GET		0
#define POST		1
#define PUT		2
#define DELETE		3

#define CHECK_VALID_ID(X,Y)	if(!valid_id(dictionary_get(data,#X))){ \
                			sprintf(result,"{\"task\":\"\",\"stauts\":\"ERROR\",\"data\":\"ID %s invalido\"}",#Y); \
                			return 0; \
        			}

struct connection_info_struct {
        int connectiontype;
        T_dictionary *data;
        struct MHD_PostProcessor *postprocessor;
};

typedef struct t_r_server {
	T_heap_task tasks_todo;
	T_bag_task tasks_done;
	T_config *config;
	struct MHD_Daemon *rest_daemon;
	pthread_t thread;
	pthread_t do_task;
	pthread_t purge_done;
	pthread_mutex_t mutex_heap_task;
	pthread_mutex_t mutex_bag_task;
	T_logs *logs;
	T_db *db;
	T_list_cloud *clouds;
	} T_rest_server;

/* Variables externas que se encuentran en controller.c,
 * el cual utiliza esta libreria. Guarda que estas variables
 * se encuentran en LA ZONA CRITICA de los hilos */
extern T_rest_server rest_server;
extern T_logs logs;

void rest_server_init(T_rest_server *r, T_db *db, T_list_cloud *cl, T_config *config);
void rest_server_add_task(T_rest_server *r, T_task *j);
void rest_server_lock(T_rest_server *r);
void rest_server_unlock(T_rest_server *r);
void rest_server_lock(T_rest_server *r);
void rest_server_unlock(T_rest_server *r);

#endif
