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
		sprintf(aux,"{\"userid\":\"%s\",\"name\":\"%s\",\"email\":\"%s\"}",row[0],row[1],row[2]);
		*size = strlen(aux) + 1;
		*data = (char *) realloc(*data,*size);
		strcpy(*data,aux);
		return 1;
	}
}

int json_task(char *status, char *id, char *result, char **message, unsigned int *size){
	/* Arma el resultado de un task */

	int total_size = (strlen(*message) + strlen(result));
	sprintf(*message,"{\"taskid\":\"%s\",\"status\":\"%s\",\"data\":",id,status);
	if(total_size > *size){
		*message = (char *)realloc(*message,total_size + 20);
		*size = total_size + 20;
	}
	strcat(*message,result);
	strcat(*message,"}");
}

int json_susc_list(char **data, int *size, MYSQL_RES *result){

	MYSQL_ROW row;
	
	strcpy(*data,"[");
	printf("Al menos aca %p\n",result);
	while ((row = mysql_fetch_row(result))){
		printf("ROW: %p\n",row);
		if((80 + strlen(*data)) > *size){
			*size = *size + 100;
			*data = (char *) realloc(*data,*size);
		}
		printf("Utilizamos el sprintf\n");
		sprintf(*data,"%s{\"id\":\"%s\",\"name\":\"%s\",\"plan_name\":\"%s\"},",*data,row[0],row[1],row[2]);
	}
	(*data)[strlen(*data)-1] = ']';
}

int json_susc_show(char **data, int *size, MYSQL_RES *result){
	/* Retorna todos los datos de una suscripcion.
	 * incluyendo los datos de las nubes */

	MYSQL_ROW row;
	char aux[1000];

	row = mysql_fetch_row(result);
	if(row==NULL){
		sprintf(aux,"{}");
		return 0;
	} else {
		sprintf(aux,"{\"suscid\":\"%s\",\"plan name\":\"%s\",\"name\":\"%s\",\"status\":\"%s\", \"services\": [",row[0],row[1],row[2],row[3]);
		*size = strlen(aux) + 1;
		*data = (char *) realloc(*data,*size);
		strcpy(*data,aux);

		/* Nube de hosting. Son los 1 datos a partir del row[4] inclusive */
		if(row[5] != NULL){
			sprintf(aux,"{\"name\":\"web_hosting\",\"hash_dir\":\"%s\",\"quota\":\"%s\",\"sites\":\"%s\"}",
			row[4],row[5],row[6]);
			*size = strlen(aux) + strlen(*data) + 4;
			*data = (char *) realloc(*data,*size);
			strcat(*data,aux);
		}
		strcat(*data,"]}");
		return 1;
	}
}

void json_site_list(char **data, int *size){
	// Toma los datos en crudo de **data y los
	// convierte en formato json
	char *aux;
	int aux_size;
	int total_size;
	char id[20];
	char name[50];
	char aux_site[100]; //Entre id,name y llaves no deben superar los 100
	int pos=0;

	aux=(char *)malloc(*size);
	strcpy(aux,*data);
	aux_size = *size;
	strcpy(*data,"{[");
	while(pos < aux_size){
		parce_data(aux,':',&pos,id);
		parce_data(aux,':',&pos,name);
		sprintf(aux_site,"{\"id\":\"%s\",\"name\":\"%s\"},",id,name);
		total_size = strlen(*data) + strlen(aux_site);
		if(*size < total_size){
			// +100 para no tener que hacer realloc en cada iteracion
			*data=(char *)realloc(*data,total_size + 100);	
			*size = total_size + 100;
		}
		strcat(*data,aux_site);
	}
	(*data)[strlen(*data)-1] = ']';
	strcat(*data,"}");
	free(aux);
}
