#include "json.h"

int json_user_list(char **data, MYSQL_RES *result){

	MYSQL_ROW row;
	int size=100;
	
	*data = (char *) realloc(*data,size);
	strcpy(*data,"[");
	while ((row = mysql_fetch_row(result))){
		if((strlen(*data)) + strlen(row[0]) + 5 > size){
			size = size + 100;
			*data = (char *) realloc(*data,size);
		}
		sprintf(*data,"%s%s,",*data,row[0]);
	}
	(*data)[strlen(*data)-1] = ']';
}

int json_user_show(char **data, MYSQL_RES *result){

	MYSQL_ROW row;
	int size=1000;	//Deberian entrar todos los datos. Sino ampliar 

	*data = (char *) realloc(*data,size);
	row = mysql_fetch_row(result);
	if(row==NULL){
		return 0;
	} else {
		sprintf(*data,"{\"userid\":\"%s\",\"name\":\"%s\",\"email\":\"%s\"}",row[0],row[1],row[2]);
		size = strlen(*data) + 1;
		*data = (char *) realloc(*data,size);
		return 1;
	}
}

int json_task(char *status, char *id, char *result, char **message){
	/* Retornamos en formato json el resultado del task */
	int size=0;
	int total_size = (strlen(*message) + strlen(result));

	sprintf(*message,"{\"taskid\":\"%s\",\"status\":\"%s\",\"data\":",id,status);
	if(total_size > size){
		size = total_size + 20;
		*message = (char *)realloc(*message,size);
	}
	strcat(*message,result);
	strcat(*message,"}");
}

int json_susc_list(char **data, MYSQL_RES *result){

	MYSQL_ROW row;
	int size=0;
	
	strcpy(*data,"[");
	printf("Al menos aca %p\n",result);
	while ((row = mysql_fetch_row(result))){
		printf("ROW: %p\n",row);
		if((80 + strlen(*data)) > size){
			size = size + 100;
			*data = (char *) realloc(*data,size);
		}
		printf("Utilizamos el sprintf\n");
		sprintf(*data,"%s{\"id\":\"%s\",\"name\":\"%s\",\"plan_name\":\"%s\"},",*data,row[0],row[1],row[2]);
	}
	(*data)[strlen(*data)-1] = ']';
}

int json_susc_show(char **data, MYSQL_RES *result){
	/* Retorna todos los datos de una suscripcion.
	 * incluyendo los datos de las nubes */

	MYSQL_ROW row;
	char aux[1000];
	int size=0;

	row = mysql_fetch_row(result);
	if(row==NULL){
		sprintf(aux,"{}");
		return 0;
	} else {
		sprintf(aux,"{\"suscid\":\"%s\",\"plan name\":\"%s\",\"name\":\"%s\",\"status\":\"%s\", \"services\": [",row[0],row[1],row[2],row[3]);
		size = strlen(aux) + 1;
		*data = (char *) realloc(*data,size);
		strcpy(*data,aux);

		/* Nube de hosting. Son los 1 datos a partir del row[4] inclusive */
		if(row[5] != NULL){
			sprintf(aux,"{\"name\":\"web_hosting\",\"hash_dir\":\"%s\",\"quota\":\"%s\",\"sites\":\"%s\"}",
			row[4],row[5],row[6]);
			size = strlen(aux) + strlen(*data) + 4;
			*data = (char *) realloc(*data,size);
			strcat(*data,aux);
		}
		strcat(*data,"]}");
		return 1;
	}
}
