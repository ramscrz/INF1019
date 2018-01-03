#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>

struct searchResult {
   char * message;
   char * indexes;
   int timesFound;
};

typedef struct searchResult SearchResult;

struct params {

	int threadid;
	int* values;
	int start;
	int end;
	int value;
};

typedef struct params Params;

void * searchInterval(void * arguments) {

	Params args = *((Params*)arguments);

	SearchResult *result = (SearchResult*) malloc(sizeof(SearchResult));
   result->timesFound = 0;
   result->indexes = (char*) malloc((args.end - args.start + 10) * sizeof(char));
   result->message = (char*) malloc(100 * sizeof(char));
	
	int k, i = 0;
	char integer[500];

	strcpy(result->indexes, "{ ");

	printf("\nThread < ID %d >: Search INTERVAL => START: %d\tEND: %d\n\n", args.threadid, 
																									args.start, 
																									args.end);
	k = 0;
	for(i = args.start; i < args.end; i++) {
		if(args.values[i] == args.value) {
			printf("Thread < ID %d > Encontrou na posição %d do vetor.\n", args.threadid, i);
			sprintf(integer, "%d", i);
   		strcat(result->indexes, integer);
			strcat(result->indexes, " ");
			(result->timesFound)++;
		} else {
			printf("Thread < ID %d > Não encontrou na posição %d ocorrência do valor.\n", args.threadid, i);
		}
	}

	strcat(result->indexes, "}");

	strcpy(result->message, "Thread < ID ");
	sprintf(integer, "%d", args.threadid);
	strcat(result->message, integer);
	strcat(result->message, " > terminou execução! ");
	strcat(result->message, "Valor ");
	sprintf(integer, "%d", args.value);
	strcat(result->message, integer);
	strcat(result->message, " foi encontrado ");
	sprintf(integer, "%d", result->timesFound);
	strcat(result->message, integer);
	strcat(result->message, " vezes!");

	return result;
}

void multiSearchValue(int iter, int nchildren, int n, int* values, int value) {

	Params ** args = (Params**) malloc(sizeof(Params*));
	
	int i, id = 0;
	int start = 0;
	int end = n;
	int resto = 0;

	pthread_t *threads = (pthread_t*) malloc(iter * sizeof(pthread_t));
	void ** results = (void**) malloc(iter * sizeof(void*));

	for(i = iter; i > 0 ; i--) {

		resto = n % nchildren;

		start = n/nchildren * (i - 1);
		end = start + n/nchildren + resto;

		n = n - resto;

		id = iter - i;

		args[id] = (Params*) malloc(sizeof(Params));

		args[id]->threadid = id + 1;
		args[id]->start = start;
		args[id]->end = end;
		args[id]->values = values;
		args[id]->value = value;
		
		pthread_create(&threads[id], NULL, searchInterval, (void*)args[id]);
	}

	for(i = 0; i < iter; i++) {

		pthread_join(threads[i], &results[i]);

		printf("\n\n%s \n%s\n\n", ((SearchResult*)results[i])->message, 
											((SearchResult*)results[i])->indexes );

	}

}

int main() {

	int values[] = {1, 6, 5, 6, 3, 5, 6, -4, 5, 6, 2, 5, 100, 26, 6, 5};
	int values2[] = {6, 2, 6, 1, 4};
	int values3[] = {6, 2, 6, 1, 6, 4};
	int values4[] = {6, 2, 6, 1, 6, 4, 6};
	int values5[] = {6, 2, 6, 1, 6, 4, 6, 21};

	pid_t pid = fork();

	if(pid != 0) {
		// Parent
		waitpid(pid, 0, 0);
		printf("\n\n<< Terceiro teste >>\n\n");
		multiSearchValue(4, 4, 6, values3, 6);

	} else {
		// Child
	
		pid = fork();

		if(pid != 0) {

			waitpid(pid, 0, 0);
			printf("\n\n<< Segundo teste >>\n\n");
      	multiSearchValue(4, 4, 16, values, 5);

		} else {
	
			printf("\n\n<< Primeiro teste >>\n\n");
   		multiSearchValue(4, 4, 16, values, 6);
		}
	}

	
	/*
	printf("\n\n<< Quarto teste >>\n\n");
	multiSearchValue(5, 5, 5, values2, 6);
	printf("\n\n<< Quinto teste >>\n\n");
	multiSearchValue(4, 4, 7, values4, 6);
	printf("\n\n<< Sexto teste >>\n\n");
	multiSearchValue(4, 4, 8, values5, 6);
	*/

	return 0;

}

