#include "types.h"
#include "stat.h"
#include "user.h"
#include "stddef.h"

int main(void){
	char testString[256] = {'\0'};
	char *fname = "README";
	printf(0, "file name: %s", fname);
	int fd = open(fname, 0);
	printf(0, "file discriptor %d", fd);
	int num = getfilename(fd, testString, 256);
	printf(1, "num is %d", num);
	printf(1, "The process ID is %d\n", getpid());
	printf(1, "the file name is %s\n", testString);
	exit();
}
