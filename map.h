#include <stdio.h>
#include <malloc.h>
#include <string.h>

#define lenA 53
#define tip const char

typedef struct pair {
	char *key;
	char *value;
} Pair;

Pair *createPair(char *key, char *value)
{ /* creeaza o pereche din hashmap */
	Pair *p = (Pair *) malloc(sizeof(Pair));

	if (!p)
		return NULL;

	p->key = (char *)calloc((strlen(key) + 1), sizeof(char));
	if (!(p->key)) {
		free(p);
		return NULL;
	}
	p->value = (char *)calloc((strlen(value) + 1), sizeof(char));
	if (p->value == NULL) {
		free(p->key);
		free(p);
		return NULL;
	}
	strcpy(p->key, key);
	strcpy(p->value, value);
	return p;
}

void removePair(Pair *p)
{ /* elibereaza memoria unei perechi */
	free(p->key);
	free(p->value);
	free(p);
}

typedef struct list {
	Pair *info;
	struct list *next;
} List, *TList;

List *createCel(char *key, char *value)
{ /* creeaza o celula din lista */
	List *l = (List *)malloc(sizeof(List));

	if (l == NULL)
		return NULL;
	l->info = createPair(key, value);
	if (l->info == NULL) {
		free(l);
		return NULL;
	}
	l->next = NULL;
	return l;
}

void removeCel(List *l)
{ /* elibereaza memoria unei celule din lista */
	removePair(l->info);
	free(l);
}

void removeL(TList *l)
{ /* elibereaza memoria unei liste */
	TList tmp, aux = *l;

	while (aux != NULL) {
		tmp = aux->next;
		removeCel(aux);
		aux = tmp;
	}
}

typedef struct map {
	TList *buckets;
	int len;
	int (*hash)(char *val);
	int (*cmp)(const char *s1, const char *s2);

} HashMap;

HashMap *init(int (*comp)(tip *, tip *),
					int (*hashFunction)(char *))
{ /* initializeaza hashmap-l */
	int i;
	HashMap *map = (HashMap *) malloc(sizeof(HashMap));

	if (!map)
		return NULL;
	map->buckets = (TList *) malloc(lenA * sizeof(TList));
	if (!map->buckets) {
		free(map);
		return NULL;
	}
	for (i = 0; i < lenA; i++)
		map->buckets[i] = NULL;

	map->len = lenA;
	map->hash = hashFunction;
	map->cmp = comp;
	return map;
}

void removeH(HashMap *map)
{ /* elibereaza memoria hashmap-ului */
	int i;

	if (!map)
		return;
	for (i = 0; i < lenA; i++)
		removeL(&(map->buckets[i]));

	free(map->buckets);
	free(map);
}

char *find(HashMap *map, char *key)
{ /* cauta valoare unei chei in hashmap */
	int pos = map->hash(key);
	TList l = map->buckets[pos];

	while (l != NULL && map->cmp(key, l->info->key) != 0)
		l = l->next;
	if (l)
		return l->info->value;
	else
		return NULL;

}

void removeKey(HashMap *map, char *key)
{ /* sterge o intrare din hashmap ce contine cheia */
	int pos = map->hash(key);
	TList aux = NULL, l = map->buckets[pos];

	while (l != NULL && map->cmp(key, l->info->key) != 0) {
		aux = l;
		l = l->next;
	}
	if (aux == NULL && l) {
		l = l->next;
		removeCel(map->buckets[pos]);
		map->buckets[pos] = l;
	} else if (l) {
		aux->next = l->next;
		removeCel(l);
	}
}

int add(HashMap *map, char *key, char *value)
{ /* adauga o intrare in hashmap */
	int pos = map->hash(key);
	TList aux = NULL, l = map->buckets[pos];
	List *cel = createCel(key, value);

	if (!cel)
		return 0;
	while (l != NULL && map->cmp(key, l->info->key) > 0) {
		aux = l;
		l = l->next;
	}
	if (aux == NULL) {
		cel->next = l;
		map->buckets[pos] = cel;
	} else {
		aux->next = cel;
		cel->next = l;
	}
	return 1;
}

int hashFunction(char *key)
{ /* funtia de hash pentru hashmap */
	if (key[0] > 'A' && key[0] < 'Z')
		return key[0] - 'A' + lenA / 2;
	else if (key[0] > 'a' && key[0] < 'z')
		return key[0] - 'a';
	else
		return lenA - 1;
}
