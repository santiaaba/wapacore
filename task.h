#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include "parce.h"
#include "json.h"
#include "dictionary.h"
#include "cloud.h"
#include "db.h"
#include "logs.h"

#ifndef JOB_H
#define JOB_H

#define TOKEN_SIZE		25
#define TASKID_SIZE		25
#define TASKRESULT_SIZE		200
#define ERROR_FATAL		"{\"code\":\"300\",\"info\":\"ERROR FATAL\"}"
#define ERROR_TASK_CLOUD	"{\"code\":\"300\",\"info\":\"TASK inexistente en la nube\"}"
#define ERROR_CLOUD		"{\"code\":\"300\",\"info\":\"Nube inaccesible\"}"

typedef enum {	
	T_NONE,
	/* CLOUDS */
	T_CLOUD_LIST,
	T_CLOUD_SHOW,

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
	T_USER_STOP,
	T_USER_START,

	/* SUSCRIPTION */
	T_SUSC_LIST,
	T_SUSC_SHOW,
	T_SUSC_ADD,
	T_SUSC_MOD,
	T_SUSC_DEL,
	T_SUSC_STOP,
	T_SUSC_START,

	/* SITES */
	T_SITE_LIST,
	T_SITE_SHOW,
	T_SITE_ADD,
	T_SITE_MOD,
	T_SITE_DEL,
	T_SITE_STOP,
	T_SITE_START,

	/*FTP_USERS */
	T_FTP_LIST,
	T_FTP_ADD,
	T_FTP_DEL,
	T_FTP_MOD,

	/* WEB INFRASTRUCTURE */
	T_HW_SERVER_LIST,
	T_HW_SERVER_SHOW,
	T_HW_SERVER_ADD,
	T_HW_SERVER_DEL,
	T_HW_SERVER_STOP,
	T_HW_SERVER_START

} T_task_type;

typedef enum {
		T_TODO,
		T_WAITING,
		T_DONE
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
/* Los task representan acciones a realizar. las TASK al crearse se crean todos
 * en estado T_TODO y con un tipo que identifica la acción a realizar. La funcion
 * task_run toma el task, identifica su tipo e invoca a la función que realiza
 * la acción. La función que realiza la acción es la que cambia el estado del
 * task a T_WAITING si se debe esperar o a T_DONE si el task ya se ha cumplido
 * En el paráemtro result queda registrado en formato JSON el resultado del task
 * incluyendo cómo termino el mismo.
 */

typedef struct {
        T_taskid id;
        T_tasktoken *token;
	T_task_user user;		//determina el usuario que es y por las acciones permitidas
	T_task_type type;		//tipo de accion a realizar
	T_task_status status;		//estado del task
	T_dictionary *data;		//datos necesarios para realizar la accion
	T_cloud *cloud;			//Cuando interviene una nube... queda asignada
	T_logs *logs;
	int step;			// Para acciones que involucran mas de un paso... indica el paso
	int result_code;		//codigo obtenido de la nube. Se inicializa en 0.
	char *result;			//resultado en formato json para retornar.
} T_task;

void task_init(T_task *t, T_tasktoken *token, T_dictionary *data, T_logs *logs);
void task_set_type(T_task *t, T_task_type type);
void task_destroy(T_task **t);
T_dictionary *task_get_data(T_task *t);
void task_run(T_task *t, T_db *db, T_list_cloud *cl);
T_tasktoken *task_get_token(T_task *t);
void task_done(T_task *t, char *message);
void task_print_status(T_task *t, char *s);
char *task_get_id(T_task *t);
T_task_status task_get_status(T_task *t);
void task_json_result(T_task *t, char **result);

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
