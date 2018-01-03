#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<fcntl.h>
#include<dirent.h>
#include<limits.h>

void printFile(char *currentPath) {

	printf("\n\nFound file! %s\n\n", currentPath);

	int file = 0;
	char r = '0';

	if(currentPath == NULL) {
		printf("\n\nError printFile: currentPath is NULL.\n\n");
		return;
	} 

	file = open(currentPath, O_RDONLY | O_CREAT | O_SYNC, 0700);

	if(file <= 0) {
		printf("\nError: file %s could not be found in this directory.\n\n", currentPath);
		exit(-1);
	}

	while(read(file, &r, 1) > 0) {
		printf("%c", r);
	}

	return;
}

int searchDirectory(char *currentPath, char *fileName) {

	DIR *dir = opendir(currentPath);
	struct dirent *directoryEnt;
	struct stat fileData;


	if(dir == NULL) {
		printf("\n\nError: Directory %s could not be opened.\n\n", currentPath);
		exit(-1);
	}

	if(stat(currentPath, &fileData) == -1) {
		printf("\n\nError searchDirectory: file does not exist in directory (1) (%s).\n\n", currentPath);
		return -1;
	}

	do {

		directoryEnt = readdir(dir);

		if(directoryEnt) {

			char filePath[PATH_MAX + 1];

			if(strcpy(filePath, currentPath) == NULL) {
				printf("\n\nError searchDirectory: strcpy failed.\n\n");
				return -1;
			}

			if(strcat(filePath, "/") == NULL) {
				printf("\n\nError searchDirectory: strcat failed.\n\n");
				return -1;
			}

			if(strcat(filePath, directoryEnt->d_name) == NULL) {
				printf("\n\nError searchDirectory: strcat failed (1).\n\n");
				return -1;
			}

			if(stat(filePath, &fileData) == -1) {
				printf("\n\nError searchDirectory: file does not exist in directory (2) (%s).\n\n", currentPath);
				return -1;
			}

			if(S_ISDIR(fileData.st_mode) && 
					strcmp(directoryEnt->d_name, ".") != 0 && 
					strcmp(directoryEnt->d_name, "..") != 0) {

				searchDirectory(filePath, fileName);
				
			} else {

				if(strcmp(directoryEnt->d_name, fileName) == 0) {

					printFile(filePath);

				}

			}
		}

	} while(directoryEnt != NULL);
	
	return 1;

}

int main(int argc, char** argv) {

	char currentPath[PATH_MAX + 1];

  if(argc != 2) {
    printf("\nInvalid number of parameters! Run: %s <NAME_OF_FILE>\n\n", argv[0]);
    exit(-1);
  }

	if(getcwd(currentPath, sizeof(currentPath)) == NULL) {
		printf("\n\nError: current working directory could not be determined by getcwd()!\n\n");
		exit(-1);
	}

	searchDirectory(currentPath, argv[1]);

	printf("\n\n");

  return 0;

}
