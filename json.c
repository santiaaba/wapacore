#include "json.h"

int json_user_list(char **data, MYSQL_RES *result){

	MYSQL_ROW row;
	int size=200;
	int a=0;
	
	*data = (char *) realloc(*data,size);
	strcpy(*data,"{\"code\":\"200\",\"info\":[");
	while ((row = mysql_fetch_row(result))){
		a++;
		//printf("Tamanos: %i, %i, %i, %i\n",strlen(*data),strlen(row[0]),strlen(row[1]),size);
		if((strlen(*data)) + strlen(row[0]) + strlen(row[1]) + strlen(row[2]) + strlen(row[3]) + 45 > size){
			//printf("Entro realloc\n");
			size = size + 200;
			*data = (char *) realloc(*data,size);
		}
		sprintf(*data,"%s{\"id\":\"%s\",\"name\":\"%s\",\"email\":\"%s\",\"status\":\"%s\"},",*data,row[0],row[1],row[2],row[3]);
	}
	if(a)
		(*data)[strlen(*data)-1] = ']';
	else
		strcat(*data,"]");
	strcat(*data,"}");
	//printf("Los usuario son: %s - %i\n",*data,strlen(*data));
}

int json_plan_list(char **data, MYSQL_RES *result){

	MYSQL_ROW row;
	int size=200;
	int a=0;
	
	*data = (char *) realloc(*data,size);
	strcpy(*data,"{\"code\":\"200\",\"info\":[");
	while ((row = mysql_fetch_row(result))){
		a++;
		if((strlen(*data)) + strlen(row[0]) + strlen(row[1]) + strlen(row[2]) + 45 > size){
			size = size + 200;
			*data = (char *) realloc(*data,size);
		}
		sprintf(*data,"%s{\"id\":\"%s\",\"name\":\"%s\",\"status\":\"%s\"},",*data,row[0],row[1],row[2]);
	}
	if(a)
		(*data)[strlen(*data)-1] = ']';
	else
		strcat(*data,"]");
	strcat(*data,"}");
}


int json_cloud_show(char **data, char *id, T_list_cloud *cl){

	T_cloud *c;

	c = list_cloud_find_id(cl,atoi(id));
	printf("QUE paso con c: %p\n",c);
	if(c){
		*data = (char *) realloc(*data,200);
		sprintf(*data,"{\"id\":%s,\"name\":\"%s\",\"type\":%i,\"status\":%i}",
			id,cloud_get_name(c),cloud_get_type(c),cloud_get_status(c));
		return 1;
	} else {
		/* No encontramos la nube. No deberia pasar */
		return 0;
	}
}

int json_cloud_list(char **data, MYSQL_RES *result, T_list_cloud *cl){
	/* Lista las nubes existentes y el estado de cada una de ellas */
	MYSQL_ROW row;
	T_cloud *c;
	int size=100;
	int a=0;

	*data = (char *) realloc(*data,size);
	strcpy(*data,"{\"code\":\"200\",\"info\":[");
	while ((row = mysql_fetch_row(result))){
		a++;
		if((strlen(*data)) + 100 > size){
			size = size + 100;
			*data = (char *) realloc(*data,size);
		}
		c = list_cloud_find_id(cl,atoi(row[0]));
		if(c){
			sprintf(*data,"%s{\"id\":%s,\"name\":\"%s\",\"type\":%s,\"status\":%i},",
				*data,row[0],row[1],row[2],cloud_get_status(c));
		} else {
			/* No encontramos la nube. No deberia pasar */
			return 0;
		}
	}
	if(a)
		(*data)[strlen(*data)-1] = ']';
	else
		strcat(*data,"]");
	strcat(*data,"}");
	return 1;
}

int json_plan_show(char **data, MYSQL_RES *result){

	MYSQL_ROW row;
	int size=1000;	//Deberian entrar todos los datos. Sino ampliar 

	*data = (char *) realloc(*data,size);
	row = mysql_fetch_row(result);
	if(row==NULL){
		return 0;
	} else {
		sprintf(*data,"{\"planid\":\"%s\",\"name\":\"%s\",\"status\":\"%s\"}",row[0],row[1],row[2]);
		size = strlen(*data) + 1;
		*data = (char *) realloc(*data,size);
		return 1;
	}
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
	/* Retorna en formato json el resultado de result */

	MYSQL_ROW row;
	int size=0;
	int a=0;
	
	size = size + 100;
	*data = (char *) realloc(*data,size);
	strcpy(*data,"{\"code\":\"200\",\"info\":[");
	while ((row = mysql_fetch_row(result))){
		a++;
		if((80 + strlen(*data)) > size){
			size = size + 100;
			*data = (char *) realloc(*data,size);
		}
		sprintf(*data,"%s{\"id\":\"%s\",\"name\":\"%s\",\"plan_name\":\"%s\",\"status\":\"%s\"},",*data,row[0],row[1],row[2],row[3]);
	}
	/* Hay que agregar en el peor de los casos dos caracteres mas que son el cierre del listado "]"
 	   y el cierrre de la estructura "}" */
	if((strlen(*data) + 2) > size){
		size = size + 2;
		*data = (char *) realloc(*data,size);
	}
	if(a)
		(*data)[strlen(*data)-1] = ']';
	else
		strcat(*data,"]");
	strcat(*data,"}");
}

int json_susc_show(char **message, MYSQL_RES *result){
	/* Toma los datos de *result y los retorna en **messaje
 	   en formato json */

	MYSQL_ROW row;
	char aux[1000];
	int size=0;

	row = mysql_fetch_row(result);
	sprintf(aux,"{\"code\":\"200\",\"info\":{\"suscid\":\"%s\",\"plan name\":\"%s\",\"name\":\"%s\",\"status\":\"%s\", \"services\": [",row[0],row[1],row[2],row[3]);
	size = strlen(aux) + 1;
	*message = (char *) realloc(*message,size);
	strcpy(*message,aux);

	/* Nube de hosting. Son los 1 datos a partir del row[4] inclusive */
	if(row[5] != NULL){
		sprintf(aux,"{\"name\":\"web_hosting\",\"hash_dir\":\"%s\",\"quota\":\"%s\",\"sites\":\"%s\"}",
		row[4],row[5],row[6]);
		size = strlen(aux) + strlen(*message) + 4;
		*message = (char *) realloc(*message,size);
		strcat(*message,aux);
	}
	strcat(*message,"]}");
}
