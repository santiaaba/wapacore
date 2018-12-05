/* La sintaxis dela rchivo de configuración debe ser
 * <variable>\t<valor>
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "logs.h"

#ifndef CONFIG_H
#define CONFIG_H

typedef struct config{
        char db_server[100];
        char db_name[20];
        char db_user[20];
        char db_pass[20];
	char logs_file[100];
	uint32_t task_done_time;	/* tiempo en segundos que un task permanece
					   en la bolsa DONE */
	T_logs_level logs_level;
} T_config;

void config_init(T_config *conf);
int config_load(const char *filename, T_config *conf);
char *config_db_server(T_config *conf);
char *config_db_name(T_config *conf);
char *config_db_user(T_config *conf);
char *config_db_pass(T_config *conf);
char *config_logs_file(T_config *conf);
uint32_t config_task_done_time(T_config *conf);
T_logs_level config_logs_level(T_config *conf);

#endif
