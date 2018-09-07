#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef JSON_H
#define JSON_H

int json_user_list(char **data, int *size, MYSQL_RES *result);
int json_user_show(char **data, int *size, MYSQL_RES *result);
int json_task(char *status, char *id, char *result, char **message, unsigned int *size);
#endif
