#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interface.h"
#include "common.h"

#include <iostream>
#include <thread>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <unistd.h>

using namespace std;

int startServer(const int portNumber);
void processCommand(const int clientSocket);

int main(int argc, char** argv) {
    if (argc != 2) {
	    fprintf(stderr,
			"usage: enter port number\n");
		exit(1);
	}
	
	int sockfd = startServer(atoi(argv[1]));
	
	struct sockaddr_storage their_addr;
	socklen_t sin_size;
	
	while(true) {
		sin_size = sizeof their_addr; 
		int client_socket = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (client_socket == -1) {
			perror("accept");
			continue;
		}
		
		cout << "accepted a request" << endl;
		
		thread t = thread(processCommand, client_socket);
		t.detach();
	}
    
    return 0;
}

void processCommand(const int clientSocket) {
    char list[8256];
    FILE* roomsRead;
    FILE* roomsWrite;
    roomsRead = fopen("rooms.dat", "r");
    roomsWrite = fopen("rooms.dat", "a");
    
    //get command from client
    char command[sizeof(Command) + 129];
    recv(clientSocket, command, sizeof(Command) + 129, 0);
    
    //process the command
    struct Answer answer;
    
    enum Command* commandEnum = (Command*) command;
    if (*commandEnum == CREATE) {
        //check if room exists
        
        fseek(roomsRead, 0, SEEK_SET);
        
        
    // } else if (*commandEnum == DELETE) {
        
    // } else if (*commandEnum == JOIN) {
        
    // } else if (*commandEnum == LIST) {
        
    } else {
        answer.status = FAILURE_UNKNOWN;
    }
    
    //answer.status = FAILURE_UNKNOWN;
    
    //send back a response to the client
    char response[sizeof(Answer) + 8256];
    memcpy(response, &answer, sizeof(Answer));
    // if (commandEnum == LIST) {
    //     strcpy(response + sizeof(Answer), list);
    // }
    send(clientSocket, response, sizeof(Answer) + 8256, 0);
    //cout << "Sent info back to client" << endl;
}

// starts the server to get it ready to accept requests from clients
// argument - portNumber => the port the server is using
// return - int => the socket for the connection
int startServer(const int portNumber) {
    struct addrinfo hints;
    struct addrinfo *serv;
    char s[INET6_ADDRSTRLEN];
    int rv;
    int sockfd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = 0;
    hints.ai_flags = AI_PASSIVE;
    
    char port[33];
    sprintf(port,"%d",portNumber);

    if ((rv = getaddrinfo(NULL, port, &hints, &serv)) != 0) {
        cerr << "getaddrinfo: " << gai_strerror(rv) << endl;
    }

    if ((sockfd = socket(serv->ai_family, serv->ai_socktype, serv->ai_protocol)) == -1) {
        perror("server: socket");
    }

    if (bind(sockfd, serv->ai_addr, serv->ai_addrlen) == -1) {
        close(sockfd);
        perror("server: bind");
    }
    freeaddrinfo(serv);

    if (listen(sockfd, 20) == -1) {
        perror("listen");
    }
    
    return sockfd;
}