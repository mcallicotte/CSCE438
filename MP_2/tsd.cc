#include <ctime>

#include <google/protobuf/timestamp.pb.h>
#include <google/protobuf/duration.pb.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <google/protobuf/util/time_util.h>
#include <grpc++/grpc++.h>

#include "sns.grpc.pb.h"

#include <iostream>
#include <pthread.h>
#include <fstream>
#include <vector>
#include <signal.h>

#include "database.h"

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
using csce438::Request;
using csce438::Reply;
using csce438::SNSService;


void serverInterruptHandler(int sig_num) {
  std::cout << std::endl << "message to stop execution, cleaning now" << std::endl;
  cleanServer();
  kill(getpid(), SIGKILL);
}


class SNSServiceImpl final : public SNSService::Service {
  
  Status List(ServerContext* context, const Request* request, Reply* reply) override {
    // ------------------------------------------------------------
    // In this function, you are to write code that handles 
    // LIST request from the user. Ensure that both the fields
    // all_users & following_users are populated
    // ------------------------------------------------------------
    return Status::OK;
  }

  Status Follow(ServerContext* context, const Request* request, Reply* reply) override {
    // ------------------------------------------------------------
    // In this function, you are to write code that handles 
    // request from a user to follow one of the existing
    // users
    // ------------------------------------------------------------
    return Status::OK; 
  }

  Status UnFollow(ServerContext* context, const Request* request, Reply* reply) override {
    // ------------------------------------------------------------
    // In this function, you are to write code that handles 
    // request from a user to unfollow one of his/her existing
    // followers
    // ------------------------------------------------------------
    return Status::OK;
  }
  
  Status Login(ServerContext* context, const Request* request, Reply* reply) override {
    // ------------------------------------------------------------
    // In this function, you are to write code that handles 
    // a new user and verify if the username is available
    // or already taken
    // ------------------------------------------------------------

    std::string username = request->username();

    auto clientIterator = clientMap.find(username);

    if (clientIterator == clientMap.end()) {
      std::cout << "adding user " << username << " to the map and files" << std::endl;
      
      //std::cout << "creating a new client" << std::endl;
      Client* newClient = new Client(username);
      //std::cout << "inserting into map right now" << std::endl;
      clientMap.insert(std::pair<std::string, Client*>(username, newClient));
      
      // clientMap.insert(std::pair<std::string, Client*>(username, new Client(username)));
      //std::cout << "successfully added to the map" << std::endl;
      //writeToClientList(username); 
      //std::cout << "successfully wrote" << std::endl;
    }

    printClientMap();
    return Status::OK;
  }

  Status Timeline(ServerContext* context, ServerReaderWriter<Message, Message>* stream) override {
    // ------------------------------------------------------------
    // In this function, you are to write code that handles 
    // receiving a message/post from a user, recording it in a file
    // and then making it available on his/her follower's streams
    // ------------------------------------------------------------
    return Status::OK;
  }

};

void RunServer(std::string port_no) {
  // ------------------------------------------------------------
  // In this function, you are to write code 
  // which would start the server, make it listen on a particular
  // port number.
  // ------------------------------------------------------------
  
  std::string serverAddress("dns:///localhost:" + port_no);
  SNSServiceImpl service;
  
  int portInt = std::stoi(port_no);

  std::cout << "about to startup server" << std::endl;
  startupServer();
  std::cout << "start grpc startup" << std::endl;
  
  signal(SIGINT, serverInterruptHandler);
  
  ServerBuilder builder;
  builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials(), &portInt);
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "server ran its start sequence." << std::endl;
  server->Wait();
  
}

int main(int argc, char** argv) {
  
  std::string port = "3010";
  int opt = 0;
  while ((opt = getopt(argc, argv, "p:")) != -1){
    switch(opt) {
      case 'p':
          port = optarg;
          break;
      default:
	         std::cerr << "Invalid Command Line Argument\n";
    }
  }
  RunServer(port);
  return 0;
}
