#include "dictionary.h"

void dictionary_init(T_dictionary *d){
	d->first=NULL;
	d->last=NULL;
	d->size=0;
}
int dictionary_add(T_dictionary *d, char *key, char *value){
	dictionary_node *new;
        dictionary_node *aux;
	int exist=0;

	printf("Agregamos al diccionario: %s = %s\n",key,value);
	aux = d->first;
	while(!exist && aux != NULL){
		if(strcmp(aux->key,key) == 0) exist=1;
		aux=aux->next;
	}
	if(!exist){
	        new = (dictionary_node*)malloc(sizeof(dictionary_node));
		new->key = (char *)malloc(strlen(key)+1);
		new->value = (char *)malloc(strlen(value)+1);
		strcpy(new->key,key);
		strcpy(new->value,value);
		new->next = NULL;
		d->size++;

		if(d->first == NULL){
			d->first = new;
			d->last = new;
		} else {
			d->last->next = new;
			d->last = new;
		}
		return 1;
	} else {
		return 0;
	}
}
char *dictionary_get(T_dictionary *d, char *key){
	dictionary_node *aux;
	int exist=0;

	aux = d->first;
	while(aux != NULL && !exist){
		if(strcmp(aux->key,key) == 0){
			exist=1;
		} else {
			aux=aux->next;
		}
	}
	if(exist)
		return aux->value;
	return NULL;
}
void dictionary_destroy(T_dictionary **d){
	dictionary_node *aux;
	
	while((*d)->first!=NULL){
		aux = (*d)->first;
		printf("Eliminamos del diccionario: %s = %s\n",aux->key,aux->value);
		free(aux->key);
		free(aux->value);
		(*d)->first = (*d)->first->next;
		free(aux);
	}
	free(*d);
}

void dictionary_clean(T_dictionary *d){
	dictionary_node *aux;

	aux = d->first;
	while(aux!=NULL){
		aux = aux->next;
		free(d->first);
		d->first = aux;
	}
}

void dictionary_print(T_dictionary *d){
	dictionary_node *aux;

	aux = d->first;
	while(aux!=NULL){
		printf("diccionario: \"%s\" = \"%s\"\n",aux->key,aux->value);
		aux = aux->next;
	}
}
