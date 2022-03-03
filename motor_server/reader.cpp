#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <errno.h>
#include <string.h>
#include <thread>
#include <chrono>

//#include <wiringPi.h>

bool OUTPUT = true;

long strtol_wrapped(char* arg);
long read_buffer(char* arg,long prev_val);

void pinMode(int pin, bool mode);
void wiringPiSetup();
void digitalWrite(int pin,int value);


int main(int argc, char** argv){
    wiringPiSetup();			// Setup the library
    pinMode(0, OUTPUT);		// Configure GPIO0 as an output


    // parse argv. Expected input is:
    int step_pin = strtol_wrapped(argv[2]);
    std::cout << "STEP PIN " << step_pin << std::endl;

    int dir_pin = strtol_wrapped(argv[3]);
    std::cout << "DIR PIN " << dir_pin << std::endl;

    long step_freq = strtol_wrapped(argv[4]);
    std::cout << "step_freq " << step_freq << std::endl;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now(); //start time measurement
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::chrono::nanoseconds step_delay = std::chrono::nanoseconds((long)(1000000000/(float)step_freq/2));
    std::chrono::nanoseconds loop_time = step_delay;
    std::chrono::nanoseconds delay_compensation = step_delay;

    std::cout << "Step_Delay " << step_delay.count() << " [ns]" << std::endl;

    long dir;

    //init pipe
    char buffer[9];
	int pipefifo, returnval;
	std::cout << "Opening Pipe" << std::endl;
	pipefifo = open(argv[1], O_RDONLY | O_NONBLOCK);
    if (pipefifo == -1){
		std::cout << "Error, cannot open fifo" << std::endl;
		throw std::exception();
	}
	char buffer_last[9];
	std::cout << "Pipe is open" << std::endl;

    //main loop
	int requested_position = 0;
//    int delay_compensation_int;

    std::this_thread::sleep_for(std::chrono::nanoseconds(1000000000));
	while(true){
//	    fflush(stdin);
        returnval = read(pipefifo, buffer, sizeof(buffer));
        if (returnval > 1){
//            std::cout << "RETURN VAL" << returnval << std::endl;
            dir = read_buffer(buffer,dir);
            std::cout << "dir: " << dir << std::endl;

        }
//        else if((returnval>0) && (returnval != 4)){
//            std::cout << "Broken order " << returnval << std::endl;
////            std::cout << strtol_wrapped(buffer) << std::endl;
//        }
//        std::cout << buffer << std::endl;
//        std::cout << "Returnval " << returnval << std::endl;
//        bool equal = (std::strcmp(charTime, buf) == 0);
//        requested_position = buffer;
//        std::cout << "CHECKPOINT B " << returnval << std::endl;
//        std::cout << "sleep for " << std::chrono::nanoseconds(10) << std::endl;

        end = std::chrono::steady_clock::now();
        loop_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin);
        delay_compensation = step_delay - (loop_time);
        if(delay_compensation.count()<0){
//            std::cout << "CKP A" << std::endl;
            std::cout << "process couldn't keep up with freq. Lost " << delay_compensation.count() << " [ns]" << std::endl;
        }
        else{
//            std::cout << "Loop completed with extra " << (delay_compensation).count() << " [ns]" << std::endl;

            std::this_thread::sleep_for(delay_compensation);
        }

        digitalWrite(step_pin,true);
        std::this_thread::sleep_for(step_delay);
        digitalWrite(step_pin,false);


        //measure elapsed time
        begin = std::chrono::steady_clock::now();
//        std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

//        std::cout << buffer << std::endl;

//        std::cout << "End Read" << std::endl;
//	    fflush(stdin);
    }

	close(pipefifo);

	return 0;
}

long strtol_wrapped(char* arg){
    char* p;
    errno = 0; // not 'int errno', because the '#include' already defined it
    long return_int = strtol(arg, &p, 10);
    if (*p != '\0' || errno != 0) {
        throw std::exception();
    }
    if (strlen(arg) == 0) {
        throw std::exception();
    }
    return return_int;

}
long read_buffer(char* arg, long prev_val){
//    std::cout << "------------------" << std::endl;
    char* p;
    errno = 0; // not 'int errno', because the '#include' already defined it
    long return_int = strtol(arg, &p, 10);

    if (strlen(arg) == 0) {
//    std::cout << "Empty String, passing" << std::endl;
        return prev_val;
    }
    if (*p != '\0' || errno != 0) {

        std::cout << "ERROR 2" << std::endl;
//        std::cout << "errno: " << errno << std::endl;
//        std::cout << "input: " << arg << std::endl;
//
//        std::cout << "return_int: " << return_int << std::endl;
//        std::cout << "p " << p << std::endl;
//        return prev_val;
    }

//    std::cout << "STRLEN: " << strlen(arg) << std::endl;

    return return_int;

}


void pinMode(int pin, bool mode){
    return;
}
void wiringPiSetup(){
    return;
}
void digitalWrite(int pin,int value){
    return;
}