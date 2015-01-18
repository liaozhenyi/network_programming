#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "../util/np_util.h"

int main(void)
{
	int fd;
	rio_t rio;
	char file[20] = "util_io";
	char buf[BUFFER_SIZE] = "";

	printf("%lu, %lu\n", sizeof(buf), strlen(buf));

	if ((fd = open(file, O_RDWR)) < 0)
		err_sys("open error");
	rio_create(0, &rio);
//	bzero(buf, BUFFER_SIZE);
//	rio_read(&rio, buf, 20);
//	fputs(buf, stdout);

	
	//bzero(buf, BUFFER_SIZE);
	while (rio_readline(&rio, buf, BUFFER_SIZE))
		fputs(buf, stdout);

	close(fd);

	return 0;
}
