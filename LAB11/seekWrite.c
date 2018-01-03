#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<fcntl.h>

struct stat fileData;

int main(int argc, char** argv) {

	FILE *file = NULL;  
	char text[1024], *buffer;
	int n = 0;
	int r;

  if(argc != 4) {
    printf("\nInvalid number of parameters! Run: %s <PATH_OF_FILE> <OFFSET> <TEXT>\n\n", argv[0]);
    exit(-1);
  }

	file = fopen(argv[1], "r");
	if(file == NULL) {
		printf("\nError: file %s could not be opened.\n\n", argv[1]);
		exit(-1);
	}

	r = getc(file);
	while(r != EOF) {
		n++;
		r = getc(file);
	}

	buffer = (char*) malloc ((n + 1)* sizeof(char));
	if(buffer == NULL) {
		printf("\nBuffer allocation error!\n");
		fclose(file);
		exit(-1);
	}

	fclose(file);

	file = fopen(argv[1], "r");
	if(file == NULL) {
		printf("\nError: file %s could not be opened.\n\n", argv[1]);
		exit(-1);
	}

	n = 0;
	r = getc(file);
	while( r != EOF) {
		buffer[n] = (char)r;
		n++;
		r = getc(file);
	}

	buffer[n] = '\0';

	fclose(file);

	if(n == 0) {
		return 0;
	}

	file = fopen(argv[1], "w+");

	if(file == NULL) {
		printf("\nError: file %s could not be opened.\n\n", argv[1]);
		exit(-1);
	}

	fputs(buffer, file);

	if(fseek(file, strtol(argv[2], NULL, 10), SEEK_SET) < 0) {
		printf("\nError: fseek() failed.\n");
		exit(-1);
	}

	if(fputs(argv[3], file) < 0) {
		printf("\nError: fputs() failed to write to file %s with offset %s.\n", argv[1], argv[2]);
		printf("\nError: %s\n", strerror(errno));		
		exit(-1);
	}

	fclose(file);

  return 0;

}
