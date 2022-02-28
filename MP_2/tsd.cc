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
#include <time.h>

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

int mainServerPID = getpid();


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
    
    client->getTimeline().printTimeline();

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
    Message message;

    //read in client username
    stream->Read(&message);

    //get client
    Client* client = clientMap.find(message.username())->second;

    // add timeline and get timeline count
    //client->openTimeline();
    //int mode = client->getTimelinesOpen();
    
    // first we print everything already in timeline
    // first first we tell client how many we are sending
    int timelineSize = client->getTimeline().getSize();
    message.set_msg(std::string(std::to_string(timelineSize)));
    stream->Write(message);
    std::cout << "sent the number of messages." << std::endl;
    client->getTimeline().printTimeline();
    int testingIncrement = 1;
    
    // Message MessageBegin, MessageLast;
    // if (client->getTimeline().getSize() != 0) {
    //   MessageBegin.set_username(std::prev(client->getTimeline().timeline.rend())->getUsername());
    //   std::cout << "Message prev(rend) name: " << MessageBegin.username() << std::endl;
      
    //   MessageLast.set_username(client->getTimeline().timeline.rbegin()->getUsername());
    //   std::cout << "Message rbegin name: " << MessageLast.username() << std::endl;
      
    // } else {
    //   std::cout << "the timeline is empty" << std::endl;
    // }
    
    if (timelineSize != 0) {
        std::cout << "starting past timeline for loop" << std::endl;
        for (auto j = client->getTimeline().timeline.rbegin(); j != std::prev(client->getTimeline().timeline.rend()); j++) {
          Message newMessage;
          std::cout << "trying to print " << testingIncrement  << " with post " << j->printPost() << std::endl;
          auto element = j;
      
          newMessage.set_username(element->getUsername());
          //std::cout << "set username: " << newMessage.username() << std::endl;

          newMessage.set_msg(element->getText());
          //std::cout << "sending this message: " << newMessage.msg() << std::endl;
          stream->Write(newMessage);
          std::cout << "sent message: " << element->printPost() << std::endl;
      
          testingIncrement++;
        }
    
    
        std::cout << "now for the last element (as in the beginning of the list)" << std::endl;
        Post lastElement = client->getTimeline().timeline.front();
        message.set_username(lastElement.getUsername());
        message.set_msg(lastElement.getText());
        stream->Write(message);
        std::cout << "sent message: " << lastElement.printPost() << std::endl;
    }
    
    
    
    
    // if (size != 0)
    
    
    std::cout << "finshed past timeline for loop, ready for new messages" << std::endl;
    
    while(stream->Read(&message)) {
      // push it to your timeline
      Post post;
      post.setUsername(message.username());
      //post.setTimestamp();
      post.setText(message.msg());
        
      std::cout << "read in a message: " << post.printPost() << std::endl;
        
      client->getTimeline().pushTimeline(post);
      //push it to your follower's timeline
      std::cout << "pushing to followers." << std::endl;
      for (auto i = client->followerMap.begin(); i != client->followerMap.end(); i++) {
        std::cout << "finding follower " << i->first << "in clientMap" <<std::endl;
        auto follower = clientMap.find(i->first);
        if (i->first != "") {
          std::cout << "pushing to their timeline" << std::endl;
          follower->second->getTimeline().pushTimeline(post);
          if (follower->second->getStreamStatus()) {
            follower->second->getStream()->Write(message);
            std::cout << "pushed to a stream" << std::endl;
          }
        }
      }
      std::cout << "pushed to to timelines" << std::endl;
    }
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
