#include "json.h"

int json_user_list(char **data, int *size, MYSQL_RES *result){

	MYSQL_ROW row;
	
	strcpy(*data,"[");
	while ((row = mysql_fetch_row(result))){
		if((40 + strlen(*data)) > *size){
			*size = *size + 100;
			*data = (char *) realloc(*data,*size);
		}
		sprintf(*data,"%s%s,",*data,row[0]);
	}
	(*data)[strlen(*data)-1] = ']';
}

int json_user_show(char **data, int *size, MYSQL_RES *result){

	MYSQL_ROW row;
	char aux[1000];

	row = mysql_fetch_row(result);
	if(row==NULL){
		sprintf(aux,"{}");
		return 0;
	} else {
		sprintf(aux,"{'userid':'%s','name':'%s','email':'%s'}",row[0],row[1],row[2]);
		*size = strlen(aux) + 1;
		*data = (char *) realloc(*data,*size);
		strcpy(*data,aux);
		return 1;
	}
}

int json_task(char *status, char *id, char *result, char **message, unsigned int *size){
	/* Arma el resultado de un task */

	int total_size = (strlen(*message) + strlen(result));
	sprintf(*message,"{\"taskid\":\"%s\",\"status\":\"%s\"}",id,status);
	if(total_size > *size){
		*message = (char *)realloc(*message,total_size + 10);
		*size = total_size + 10;
	}
	strcat(*message,result);
	strcat(*message,"\"}");
}
