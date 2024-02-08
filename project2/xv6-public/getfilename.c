#include "types.h"
#include "stat.h"
#include "user.h"
#include "stddef.h"

int main(int argc, char *argv[]){
	if (argc == 1){
		printf(0, "USAGE: getfilename <name_of_file>\n");
		exit();
	}
	char filenameOut[256] = {'\0'};
	char *filenameIn = argv[1];
	int fd;

	fd = open(filenameIn,0);
       	int suc = getfilename(fd, filenameOut, 256);
	if (suc ==0 ){
		printf(0, "XV6_TEST_OUTPUT Open filename: %s\n", filenameIn);
	} else {
		printf(0, "ERROR: File not found");
	}

	exit();
	
}

