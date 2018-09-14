#include <stdio.h>
#include <pthread.h>
#include "parce.h"
#include "json.h"
#include "dictionary.h"
#include "cloud.h"
#include <string.h>
#include "db.h"
#include <time.h>

#ifndef JOB_H
#define JOB_H

#define TOKEN_SIZE		25
#define TASKID_SIZE		25
#define TASKRESULT_SIZE		200

typedef enum {	
	/* PLAN */
	T_PLAN_LIST,
	T_PLAN_SHOW,
	T_PLAN_ADD,
	T_PLAN_MOD,
	T_PLAN_DEL,
	
	/* USER */
	T_USER_LIST,
	T_USER_SHOW,
	T_USER_ADD,
	T_USER_MOD,
	T_USER_DEL,

	/* SUSCRIPTION */
	T_SUSC_LIST,
	T_SUSC_SHOW,
	T_SUSC_ADD,
	T_SUSC_MOD,
	T_SUSC_DEL,

	/* SUSCRIPTION */
	T_SITE_LIST,
	T_SITE_SHOW,
	T_SITE_ADD,
	T_SITE_MOD,
	T_SITE_DEL

} T_task_type;

typedef enum {
		T_TODO,
		T_WAITING,
		T_DONE_OK,
		T_DONE_ERROR
} T_task_status;

typedef enum {
		T_ADMIN,
		T_TENNANT
} T_task_user;

typedef struct heap_task T_heap_task;
typedef struct bag_task T_bag_task;

typedef char T_taskid[TASKID_SIZE];
typedef char T_tasktoken[TOKEN_SIZE];


/*****************************
		TASK	
******************************/
typedef struct {
        T_taskid id;
        T_tasktoken *token;
	T_task_user user;		//determina el usuario que es y por las acciones permitidas
	T_task_type type;		//tipo de accion a realizar
	T_task_status status;		//estado del task
	T_dictionary *data;		//datos necesarios para realizar la accion
	T_cloud *cloud;			//Cuando interviene una nube... queda asignada
	char *result;			//resultado en formato json para retornar.
	unsigned int result_size;	//datos para realizar la accion
} T_task;

void task_init(T_task *t, T_tasktoken *token, T_task_type type, T_dictionary *data);
void task_destroy(T_task **t);
void task_run(T_task *t, T_db *db, T_list_cloud *cl);
T_tasktoken *task_get_token(T_task *t);
void task_print_status(T_task *t, char *s);
char *task_get_id(T_task *t);
T_task_status task_get_status(T_task *t);
char *task_get_result(T_task *t);

/* USERS */
int task_user_list(T_task *t, T_db *db);
int task_user_show(T_task *t, T_db *db);
int task_user_add(T_task *t, T_db *db);

/* SUSCRIPTION */
int task_susc_add(T_task *t, T_db *db);

/* Sites */
int task_site_list(T_task *t);
int task_site_show(T_task *t);
int task_site_add(T_task *t);
int task_site_del(T_task *t);
int task_site_mod(T_task *t);

/*****************************
         Cola de tareas
******************************/
typedef struct j_h_node {
        T_task *data;
        struct j_h_node *next;
} heap_t_node;

struct heap_task {
        unsigned int size;
        heap_t_node *first;
        heap_t_node *last;
};

void heap_task_init(T_heap_task *h);
void heap_task_push(T_heap_task *h, T_task *t);
T_task *heap_task_exist(T_heap_task *h, T_taskid id);
T_task *heap_task_pop(T_heap_task *h);
unsigned int heap_task_size(T_heap_task *h);
void heap_task_print(T_heap_task *h);

/****************************
 	BAG Jobs
*****************************/

typedef struct j_b_node {
        T_task *data;
        struct j_b_node *next;
} bag_t_node;

struct bag_task {
        unsigned int size;
        bag_t_node *first;
        bag_t_node *last;
        bag_t_node *actual;
};

void bag_task_init(T_bag_task *b);
void bag_task_add(T_bag_task *b, T_task *t);
T_task *bag_task_pop(T_bag_task *b, T_taskid *id);
unsigned int bag_task_size(T_bag_task *b);
void bag_task_print(T_bag_task *b);

#define JOB_H
#endif
