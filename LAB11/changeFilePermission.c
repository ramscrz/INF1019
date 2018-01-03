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

  if(argc != 3) {
    printf("\nInvalid number of parameters! Run: %s <PATH_OF_DIRECTORY/FILE> <NEW_PERMISSION>\n\n", argv[0]);
    exit(-1);
  }

	if(stat(argv[1], &fileData) == -1) {
		printf("\nError: file %s does not exist.\n\n", argv[1]);
		exit(-1);
	}

	printf("\n==============================================\n");
	printf("\n\tPermission of File: %s\n", argv[1]);
	printf("\n==============================================\n");

	if(fileData.st_mode & S_IRWXU) {
		printf("\nPermission: Read, write, execute/search by owner.\n\n");
	} 
	if(fileData.st_mode & S_IRWXG) {
		printf("\nPermission: Read, write, execute/search by group.\n\n");
	}
	if(fileData.st_mode & S_IRWXO) {
		printf("\nPermission: Read, write, execute/search by others.\n\n");
	}

	printf("\nPermission: %x", fileData.st_mode);

	printf("\n\n==============================================\n\n");
	printf("\nAfter changing file permission to %s...\n\n", argv[2]);


	if(strcmp(argv[2], "S_IRWXU") == 0) {
		chmod(argv[1], S_IRWXU);
	} else if(strcmp(argv[2], "S_IRWXG") == 0) {
		chmod(argv[1], S_IRWXG);
	} else if(strcmp(argv[2], "S_IRWXO") == 0) {
		chmod(argv[1], S_IRWXO);
	} else if(strcmp(argv[2], "S_IRWXU|S_IRWXG") == 0) {
		chmod(argv[1], S_IRWXU | S_IRWXG);
	} else if(strcmp(argv[2], "S_IRWXG|S_IRWXU") == 0) {
		chmod(argv[1], S_IRWXU | S_IRWXG);
	} else if(strcmp(argv[2], "S_IRWXU|S_IRWXO") == 0) {
		chmod(argv[1], S_IRWXU | S_IRWXO);
	} else if(strcmp(argv[2], "S_IRWXO|S_IRWXU") == 0) {
		chmod(argv[1], S_IRWXU | S_IRWXO);
	} else if(strcmp(argv[2], "S_IRWXG|S_IRWXO") == 0) {
		chmod(argv[1], S_IRWXG | S_IRWXO);
	} else if(strcmp(argv[2], "S_IRWXO|S_IRWXG") == 0) {
		chmod(argv[1], S_IRWXG | S_IRWXO);
	} else if(strcmp(argv[2], "S_IRWXO|S_IRWXG|S_IRWXU") == 0) {
		chmod(argv[1], S_IRWXO | S_IRWXG | S_IRWXU);
	} else if(strcmp(argv[2], "S_IRWXG|S_IRWXO|S_IRWXU") == 0) {
		chmod(argv[1], S_IRWXO | S_IRWXG | S_IRWXU);
	} else if(strcmp(argv[2], "S_IRWXO|S_IRWXU|S_IRWXG") == 0) {
		chmod(argv[1], S_IRWXO | S_IRWXG | S_IRWXU);
	} else if(strcmp(argv[2], "S_IRWXU|S_IRWXO|S_IRWXG") == 0) {
		chmod(argv[1], S_IRWXO | S_IRWXG | S_IRWXU);
	} else if(strcmp(argv[2], "S_IRWXU|S_IRWXG|S_IRWXO") == 0) {
		chmod(argv[1], S_IRWXO | S_IRWXG | S_IRWXU);
	} else if(strcmp(argv[2], "S_IRWXG|S_IRWXU|S_IRWXO") == 0) {
		chmod(argv[1], S_IRWXO | S_IRWXG | S_IRWXU);
	} else {
		printf("\nPermission %s not recognized!\n\n", argv[2]);
	}

	if(stat(argv[1], &fileData) == -1) {
		printf("\nError: file %s does not exist.\n\n", argv[1]);
		exit(-1);
	}

	printf("\n==============================================\n");
	printf("\n\tPermission of File: %s\n", argv[1]);
	printf("\n==============================================\n");

	if(fileData.st_mode & S_IRWXU) {
		printf("\nPermission: Read, write, execute/search by owner.\n\n");
	}

	if(fileData.st_mode & S_IRWXG) {
		printf("\nPermission: Read, write, execute/search by group.\n\n");
	} 

	if(fileData.st_mode & S_IRWXO) {
		printf("\nPermission: Read, write, execute/search by others.\n\n");
	}

	printf("\nPermission: %x", fileData.st_mode);

	printf("\n\n==============================================\n\n");

  return 0;
}
