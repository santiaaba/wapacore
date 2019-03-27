#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cloud.h"

#ifndef JSON_H
#define JSON_H

int json_task(char *status, char *id, char *result, char **message);

int json_user_list(char **data, MYSQL_RES *result);
int json_user_show(char **data, MYSQL_RES *result);

int json_plan_list(char **data, MYSQL_RES *result);
int json_plan_show(char **data, MYSQL_RES *result);

int json_cloud_list(char **data, MYSQL_RES *result, T_list_cloud *cl);
int json_cloud_show(char **data, char *id, T_list_cloud *cl);

int json_susc_list(char **data, MYSQL_RES *result);
int json_susc_show(char **data, MYSQL_RES *result);

#endif
