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
// using csce438::Message;
// using csce438::ListReply;
// using csce438::Request;
// using csce438::Reply;
// using csce438::SNSService;

enum Type {MASTER, SLAVE, SYCHRONIZER, COORDINATOR};

struct ClusterServer {
	int clusterNum;
	Type type;
	std::string port;
	ServerReaderWriter<oc::HeartBeat, oc::HeartBeat>* stream = 0; 

	ClusterServer(int num, Type theType, std::string thePort): clusterNum{num}, type{theType}, port{thePort} {}
};

struct Client {
	int username; 
	int clusterNum;

	Client(int name): username{name} {
		clusterNum = (name % 3);
	}
};

struct Cluster {
	std::vector<Client> clients = std::vector<Client>();
	ClusterServer *master, *slave, *sychronizer;

	~Cluster() { delete master; delete slave; delete sychronizer; }
};

std::vector<Cluster> clusters(3);

class CoordServiceImpl final : public oc::CoordService::Service {
	Status Login(ServerContext* context, const oc::Request* request, oc::Reply* reply) override {
		if (request->requester() == oc::CLIENT) {
			Client client(request->id());
			clusters[client.clusterNum].clients.push_back(client);
			std::string message = "Added client " + std::to_string(client.username) + " to cluster " + std::to_string(client.clusterNum);
			reply->set_msg(message);
		} else { // this is a server
			// if (request.server_type() == ServerType::MASTER) {
			// 	clusters[request.id()].master = Server(request.id(), Type::MASTER, request.port_number());
			// }
		}
		return Status::OK;
	}

	Status ServerCommunicate(ServerContext* context, ServerReaderWriter<oc::HeartBeat, oc::HeartBeat>* stream) override {
		//unimplemented
		return Status::OK;
	}
};

// runs the client facing and server facing coordinator services as different threads
void runCoord(std::string port) {
	std::string coordAddress = "0.0.0.0:" + port;
	CoordServiceImpl service;

	ServerBuilder builder;
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