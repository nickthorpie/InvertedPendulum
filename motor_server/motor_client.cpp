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

//    long dir;
    long step_requested = 0;
	long step_current = 0;
	long step_diff;
	int step;

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

    std::this_thread::sleep_for(std::chrono::nanoseconds(1000000000));
    //main loop
	while(true){
	    //check fifo
        returnval = read(pipefifo, buffer, sizeof(buffer));
        //if fifo has been updated, pull new position request from buffer
        if (returnval > 1){
            step_requested = read_buffer(buffer,step_requested);
            std::cout << "step_requested: " << step_requested << std::endl;

            // Move direction code in here so it only updates when there's
            // a potential new direction request
            step_diff = (step_requested - step_current);
            if (step_diff>0){
                step = 1;
                digitalWrite(step_pin,false);
            }else{
                digitalWrite(step_pin,true);
                step = -1;
            }

        }else{
            // need to update step diff every step regardless. not sure if its much Faster
            // than keeping it in the main loop
            step_diff = (step_requested - step_current);
        }

        // check if we are at our destination, don't do anything
        if(step_diff==0){
                // should I put a delay?
                continue;
            }

        // delay compensation to ensure loop is consistent with requested frequency
        end = std::chrono::steady_clock::now();
        loop_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin);
        delay_compensation = step_delay - (loop_time);
        if(delay_compensation.count()<0){ //warn user if lagging behind.
            std::cout << "process couldn't keep up with freq. Lost " << delay_compensation.count() << " [ns]" << std::endl;
        }
        else{ //if all is well, delay to chill out
            std::this_thread::sleep_for(delay_compensation);
        }
        // step motor once
        digitalWrite(step_pin,true);
        std::this_thread::sleep_for(step_delay);
        digitalWrite(step_pin,false);

        //update counter
        step_current = step_current + step;

        //measure elapsed time
        begin = std::chrono::steady_clock::now();
        std::cout << "new position" << step_current << std::endl;
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