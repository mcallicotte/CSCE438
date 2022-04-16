// this is the coordinator file

#include <ctime>

#include <google/protobuf/timestamp.pb.h>
#include <google/protobuf/duration.pb.h>

#include <iostream>
#include <memory>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <google/protobuf/util/time_util.h>
#include <grpc++/grpc++.h>

#include "coord.grpc.pb.h"

using google::protobuf::Timestamp;
using google::protobuf::Duration;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;
using csce438::Message;
using csce438::ListReply;
using csce438::Request;
using csce438::Reply;
using csce438::SNSService;

enum Type {master, slave, sychronizer}

struct Server {
	int clusterNum;
	Type type;
	std::string port;
	ServerReaderWriter<Heartbeat, Heartbeat>* stream = 0; 

	Server(int num, Type theType, std::string thePort): clusterNum{num}, type{theType}, port{thePort} { }
}

struct Client {
	std::string username; 
	int clusterNum;

	Client(std::string name): username{name} {
		int convert = atoi(name);
		clusterNum = (convert % 3);
	}
}

struct Cluster {
	std::vector<Client> clients;
	Server master, slave, sychronizer;
}

std::vector<Cluster> clusters(3);

class CoordServiceImpl final : public CoordService::Service {
	Status Login(ServerContext* context, const Request* request, Reply* reply) override {

	}
}

// runs the client facing and server facing coordinator services as different threads
void runCoord(std::string port) {
	std::string coordAddress = "0.0.0.0:" + port;
	SNSServiceImpl service;

	ServerBuilding builder;
	builder.AddListeningPort(coordAddress, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);
	std::unique_ptr<Server> server(builder.BuildAndStart());
	std::cout << "Coordinator listening on " << coordAddress << std::endl;

	server->Wait();
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