#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char** argv){
	int pipefifo, returnval;
	char buffer[9] = "TESTDATA";
    fifo_dir = argv[1];
	returnval = mkfifo(fifo_dir, 0666);

	pipefifo = open("/tmp/myfifo", O_WRONLY);
	if (pipefifo == -1)
	{
		std::cout << "Error, cannot open fifo" << std::endl;
		return 1;
	}

	write(pipefifo, buffer, sizeof(buffer));

	close(pipefifo);

	return 0;

}