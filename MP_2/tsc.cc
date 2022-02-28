#include <iostream>
#include <string>
#include <unistd.h>
#include <grpc++/grpc++.h>
#include "client.h"

#include "sns.grpc.pb.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <thread>

#include <chrono>
#include <ctime>
#include <signal.h>
#include <unistd.h>



class Client : public IClient
{
    public:
        Client(const std::string& hname,
               const std::string& uname,
               const std::string& p)
            :hostname(hname), username(uname), port(p)
            {}
        std::unique_ptr<csce438::SNSService::Stub> stub_;
    protected:
        virtual int connectTo();
        virtual IReply processCommand(std::string& input);
        virtual void processTimeline();
        //static void otherTimeline(Client newClient);
    public:
        std::string hostname;
        std::string username;
        std::string port;
        
        // You can have an instance of the client stub
        // as a member variable.
        
};

void readTimeline(Client* client);
void writeTimeline(Client* client, grpc::ClientReaderWriter<csce438::Message, csce438::Message>* stream);

int main(int argc, char** argv) {

    std::string hostname = "localhost";
    std::string username = "default";
    std::string port = "3010";
    int opt = 0;
    while ((opt = getopt(argc, argv, "h:u:p:")) != -1){
        switch(opt) {
            case 'h':
                hostname = optarg;break;
            case 'u':
                username = optarg;break;
            case 'p':
                port = optarg;break;
            default:
                std::cerr << "Invalid Command Line Argument\n";
        }
    }

    Client myc(hostname, username, port);
    // You MUST invoke "run_client" function to start business logic
    myc.run_client();

    return 0;
}

int Client::connectTo()
{
	// ------------------------------------------------------------
    // In this function, you are supposed to create a stub so that
    // you call service methods in the processCommand/porcessTimeline
    // functions. That is, the stub should be accessible when you want
    // to call any service methods in those functions.
    // I recommend you to have the stub as
    // a member variable in your own Client class.
    // Please refer to gRpc tutorial how to create a stub.
	// ------------------------------------------------------------
	
	stub_ = csce438::SNSService::NewStub(grpc::CreateChannel(hostname + ":" + port, grpc::InsecureChannelCredentials()));
	//std::cout << "client ran connectTo" << std::endl;

    csce438::Request loginRequest;
    loginRequest.set_username(username);

    csce438::Reply loginReply;

    grpc::ClientContext context;

    grpc::Status status = stub_->Login(&context, loginRequest, &loginReply);

    //std::cout << "tried to log in" << std::endl;

    return 1; // return 1 if success, otherwise return -1
}

IReply Client::processCommand(std::string& input)
{
	// ------------------------------------------------------------
	// GUIDE 1:
	// In this function, you are supposed to parse the given input
    // command and create your own message so that you call an 
    // appropriate service method. The input command will be one
    // of the followings:
	//
	// FOLLOW <username>
	// UNFOLLOW <username>
	// LIST
    // TIMELINE
	//
	// ------------------------------------------------------------
	
	std::stringstream sstream;
    sstream << input;

    std::string command, othername = "";
    sstream >> command;
    sstream >> othername;

    csce438::Request commandRequest;
    commandRequest.set_username(username);
    

    csce438::Reply commandReply;

    grpc::ClientContext context;

    grpc::Status status;

    //commandRequest.set_arguments(command);
    

    if (command == "FOLLOW" || command == "UNFOLLOW") {
        commandRequest.add_arguments(othername);
        if (command == "FOLLOW") {
            status = stub_->Follow(&context, commandRequest, &commandReply);
        } else {
            status = stub_->UnFollow(&context, commandRequest, &commandReply);
        }
    } else {
        if (command == "LIST") {
            status = stub_->List(&context, commandRequest, &commandReply);
        } else if (command == "TIMELINE") {
            // this will run timeline
            //std::cout << "timeline is not implemented" << std::endl;
            // auto writer(stub_->Timeline(&context, &stats))
            commandReply.set_msg("1");
        } else {
            status = grpc::Status::OK;  // failure_invalid_command
            commandReply.set_msg("5");
        }
    }

    // ------------------------------------------------------------
	// GUIDE 2:
	// Then, you should create a variable of IReply structure
	// provided by the client.h and initialize it according to
	// the result. Finally you can finish this function by returning
    // the IReply.
	// ------------------------------------------------------------

    IReply ir;

    ir.grpc_status = status;
    // if (status.ok()) {
    //     ir.comm_status = SUCCESS;
    // } else if (status.error_code() == grpc::StatusCode::FAILED_PRECONDITION) {
    //     ir.comm_status = FAILURE_NOT_EXISTS;
    // } else if (status.error_code() == grpc::StatusCode::ALREADY_EXISTS) {
    //     ir.comm_status = FAILURE_ALREADY_EXISTS;
    // } else if (status.error_code() == grpc::StatusCode::INVALID_ARGUMENT) {
    //     ir.comm_status = FAILURE_INVALID_USERNAME;
    // } else if (status.error_code() == grpc::StatusCode::CANCELLED) {
    //     ir.comm_status = FAILURE_INVALID;
    // } else {
    //     ir.comm_status = FAILURE_UNKNOWN;
    // }
    
    std::string message = commandReply.msg();
    
    if (message == "1") {
        ir.comm_status = SUCCESS;
    } else if(message == "2") {
        ir.comm_status = FAILURE_ALREADY_EXISTS;
    } else if (message == "3") {
        ir.comm_status = FAILURE_NOT_EXISTS;
    } else if (message == "4") {
        ir.comm_status = FAILURE_INVALID_USERNAME;
    } else if (message == "5") {
        ir.comm_status = FAILURE_INVALID;
    } else {
        ir.comm_status = FAILURE_UNKNOWN;
    }
    
    

    if (command == "LIST") {
        //add the list stuff to the ireplay
        int totalUsers = commandReply.all_users_size();
        std::vector<std::string> allUsers;
        for (int i = 0; i < totalUsers; i++) {
            allUsers.push_back(commandReply.all_users(i));
        }

        ir.all_users = allUsers;

        int totalFollowers = commandReply.following_users_size();
        std::vector<std::string> allFollows;
        allFollows.push_back(username);
        for (int i = 0; i < totalFollowers; i++) {
            allFollows.push_back(commandReply.following_users(i));
        }

        ir.following_users = allFollows;
    }

    return ir;
    
	// ------------------------------------------------------------
    // HINT: How to set the IReply?
    // Suppose you have "Follow" service method for FOLLOW command,
    // IReply can be set as follow:
    // 
    //     // some codes for creating/initializing parameters for
    //     // service method
    //     IReply ire;
    //     grpc::Status status = stub_->Follow(&context, /* some parameters */);
    //     ire.grpc_status = status;
    //     if (status.ok()) {
    //         ire.comm_status = SUCCESS;
    //     } else {
    //         ire.comm_status = FAILURE_NOT_EXISTS;
    //     }
    //      
    //      return ire;
    // 
    // IMPORTANT: 
    // For the command "LIST", you should set both "all_users" and 
    // "following_users" member variable of IReply.
    // ------------------------------------------------------------
}

void Client::processTimeline()
{
	// ------------------------------------------------------------
    // In this function, you are supposed to get into timeline mode.
    // You may need to call a service method to communicate with
    // the server. Use getPostMessage/displayPostMessage functions
    // for both getting and displaying messages in timeline mode.
    // You should use them as you did in hw1.
	// ------------------------------------------------------------

    grpc::ClientContext context;
    csce438::Message message;
    
    // std::thread i = std::thread(initializeTimeline, this);
    // i.join();
    
    std::thread r = std::thread(readTimeline, this);
    r.join();
    
    // std::thread w = std::thread(writeTimeline, this);
    // w.detach();

    // ------------------------------------------------------------
    // IMPORTANT NOTICE:
    //
    // Once a user enter to timeline mode , there is no way
    // to command mode. You don't have to worry about this situation,
    // and you can terminate the client program by pressing
    // CTRL-C (SIGINT)
	// ------------------------------------------------------------
}
//};


void readTimeline(Client* client) {
    grpc::ClientContext context;
    csce438::Message message;
    
    std::cout << "opening read timeline" << std::endl;
    std::shared_ptr<grpc::ClientReaderWriter<csce438::Message, csce438::Message>> reader(client->stub_->Timeline(&context));
        
    std::cout << "initialize timeline" << std::endl;
    
    message.set_username(client->username);
    reader->Write(message);
        
    //first read in number to read in
    reader->Read(&message);
    int until = std::stoi(message.msg());
    int i = 0;
    while (i < until) {
        std::cout << "waiting for a message" << std::endl;
        csce438::Message newMessage;
        reader->Read(&newMessage);
        time_t currentTime; 
        time(&currentTime);
        if (message.msg() != "") {
            displayPostMessage(newMessage.username(), newMessage.msg(), currentTime);
        }
        i++;
        std::cout << "recieved message" << std::endl;
    }
    
    std::cout << "finished initializing" << std::endl;
    
    std::thread w = std::thread([reader]() {
        grpc::ClientContext context;
        csce438::Message message;
    
        //open write timeline
        std::cout << "you opened the writer" << std::endl;

     while (reader->Read(&message)) {
            std::cout << "recieved a message" << std::endl;
            time_t currentTime; 
            time(&currentTime);
            displayPostMessage(message.username(), message.msg(), currentTime);
            std::cout << "printed message" << std::endl;
        }
    });
    w.detach();
    
    while(true) {
        std::string fullMessage = getPostMessage();
        std::cout << "wrote a message" << std::endl;
        message.set_username(client->username);
        message.set_msg(fullMessage);
    
        reader->Write(message);
        std::cout << "sent message to server" << std::endl;
    }
}
