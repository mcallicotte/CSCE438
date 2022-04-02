// this is the coordinator file

#include <ctime>

#include <google/protobuf/timestamp.pb.h>
#include <google/protobuf/duration.pb.h>

#include <iostream>
#include <memory>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <google/protobuf/util/time_util.h>
#include <grpc++/grpc++.h>

#include "sns.grpc.pb.h"

void serverService() {  /*unimplemented*/ }

void clientService() { }

// runs the client facing and server facing coordinator services as different threads
void runCoord(std::string port) {
	
}


// main takes in the command line arguments, then runs coordinator
int main(int argc, char** argv) {
	std::string port = "3010";
	int opt = 0;
	while ((opt = getopt(argc, argv, "p:")) != -1) {
		switch(opt) {
			case 'p':
				port = optarg;
				break;
			default:
				std::cerr << "Invalid Command Line Argument\n";
		}
	}
	runCoord(port);
	
	return 0;
}