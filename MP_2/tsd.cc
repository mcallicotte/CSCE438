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
#include "sns.pb.h"

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
    // std::cout << "ran list - not implemented" << std::endl;
    // return Status(grpc::StatusCode::UNIMPLEMENTED, "m1");

    std::string username = request->username();

    // get all users
    for (auto i = clientMap.begin(); i != clientMap.end(); i++) {
      reply->add_all_users(i->second->getUsername());
    }

    // get followers
    auto iter = clientMap.find(username);
    for (auto i = iter->second->followerMap.begin(); i != iter->second->followerMap.end(); i++) {
      if (i->first != "") {
        reply->add_following_users(i->first);
      }
    }

    reply->set_msg("1");
    return Status::OK;
  }

  Status Follow(ServerContext* context, const Request* request, Reply* reply) override {
    // ------------------------------------------------------------
    // In this function, you are to write code that handles 
    // request from a user to follow one of the existing
    // users
    // ------------------------------------------------------------
    
    std::string username = request->username();
    const std::string fname = request->arguments(0);

    if (username == fname) {
      reply->set_msg("2");
      return Status::OK;//(grpc::StatusCode::ALREADY_EXISTS, "m2");
    }

    auto iter = clientMap.find(fname);
    if (iter == clientMap.end()) {
      reply->set_msg("3");
      return Status::OK;//(grpc::StatusCode::FAILED_PRECONDITION, "m3");
    }
    
    Client* client = iter->second;
    bool status = client->addFollower(username);

    if (status) {
      reply->set_msg("1");
      return Status::OK;
    } else {
      reply->set_msg("2");
      return Status::OK;//(grpc::StatusCode::ALREADY_EXISTS, "m4");
    } 
  }

  Status UnFollow(ServerContext* context, const Request* request, Reply* reply) override {
    // ------------------------------------------------------------
    // In this function, you are to write code that handles 
    // request from a user to unfollow one of his/her existing
    // followers
    // ------------------------------------------------------------
    // std::cout << "ran unfollow - not implemented" << std::endl;
    // return Status::UNIMPLEMENTED;

    std::string username = request->username();
    const std::string fname = request->arguments(0);

    if (username == fname) {
      reply->set_msg("3");
      return Status::OK;//(grpc::StatusCode::INVALID_ARGUMENT, "m5");
    }

    auto iter = clientMap.find(fname);
    if (iter == clientMap.end()) {
      reply->set_msg("3");
      return Status::OK;//(grpc::StatusCode::FAILED_PRECONDITION, "m6");
    }
    
    Client* client = iter->second;
    bool status = client->removeFollower(username);

    if (status) {
      reply->set_msg("1");
      return Status::OK;
    } else {
      reply->set_msg("3");
      return Status::OK;//(grpc::StatusCode::INVALID_ARGUMENT, "m7");
    } 

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
