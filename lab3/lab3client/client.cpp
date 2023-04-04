#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

#include <signal.h>

#include "bbs.h"

#define SEED 866
#define P 7
#define Q 263

bool stop_sig_received = 0;

void sig_handler(int sig){
	std::cout<<"stopsig received"<<std::endl;
	stop_sig_received = true;
};

void set_gen_params(bbs::BBSParams *prms, int fd){
	std::cout<<"Trying to set generation parameters"<<std::endl;
	if (int err = devctl(fd,_SET_PARAMS, prms, sizeof(*prms), NULL) != EOK) {
		std::cerr << "An error occurred while setting generation parameters\n" << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
	else {
		std::cout << "Success\n"<<std::endl;
	}
};

void get_elem(std::uint32_t *elem, int fd){

	if (int err = devctl(fd,_GET_ELEM, elem, sizeof(std::uint32_t),NULL) != EOK){
		std::cerr << "An error occurred while getting prs element\n" << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	};
}

int main( int argc, char **argv )
{
	//set "stop" signal handler
	signal(SIGINT, sig_handler);

    // open a connection to the server (fd == coid)
    int fd = open("/dev/cryptobbs", O_RDWR);
    if(fd < 0)
    {
        std::cerr << "E: unable to open server connection: " << strerror(errno ) << std::endl;
        return EXIT_FAILURE;
    }

    bbs::BBSParams param(SEED, P, Q);

    set_gen_params(&param, fd);

    std::vector <std::uint32_t> vec (1024);
    std::uint32_t el;
    std::uint32_t pointer = 0;

    while (!stop_sig_received){

    	get_elem(&el, fd);
    	vec.at(pointer) = el;
    	++pointer;
    	if (pointer == 1024){
    		pointer = 0;
    	};

    };

    std::cout << "PRS: " << std::endl;
    for (auto &_el: vec)
    std::cout << _el << std::endl;
    std::cout <<"len = "<< vec.size();

    close(fd);

    return EXIT_SUCCESS;
}



