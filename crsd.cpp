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
#include "datastructs.h"

#include <iostream>
#include <thread>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <unistd.h>
#include <map>


using namespace std;

int startServer(const int portNumber);
int acceptRequest(const int sockfd);
void processCommand(const int clientSocket, PortFinder* portFinder, map<string, Room>* rooms);
bool findRoomName(/*FILE* stream,*/ char* name);
void addToRoomsFile(FILE* stream, char* name);
void roomThread(const int port, PortFinder* portFinder);
void startRooms();

int main(int argc, char** argv) {
    if (argc != 2) {
	    fprintf(stderr,
			"usage: enter port number\n");
		exit(1);
	}
	
	PortFinder portFinder(atoi(argv[1]));
	map<string, Room> rooms;
	
	int sockfd = startServer(atoi(argv[1]));
	
	while(true) {
		int client_socket = acceptRequest(sockfd);
		if (client_socket == -1) {
		    perror("accept");
		    continue;
		}
		cout << "accepted a request" << endl;
		
		thread t = thread(processCommand, client_socket, &portFinder, &rooms);
		t.detach();
	}
    
    return 0;
}

void processCommand(const int clientSocket, PortFinder* portFinder, map<string, Room>* rooms) {
    char list[8256];
    FILE* roomsRead;
    FILE* roomsWrite;
    //roomsRead = fopen("rooms.dat", "r");
    roomsWrite = fopen("rooms.dat", "a");
    
    //get command from client
    char command[sizeof(Command) + 129];
    recv(clientSocket, command, sizeof(Command) + 129, 0);
    
    //process the command
    struct Answer answer;
    
    enum Command* commandEnum = (Command*) command;
    if (*commandEnum == UNKNOWN) {
        answer.status = FAILURE_UNKNOWN;
    } else if (*commandEnum == LIST) {
    
        
    } else {
        //parse out name information
        char name[127];
        memcpy(name, command + sizeof(Command), 126);
        name[126] = '\0';
        
        if (name[0] == '\0') {
            // the name isn't valid
            answer.status = FAILURE_INVALID;
        } else {
            string nameString = name;
            bool roomFound;
            auto roomIter = rooms->find(nameString);
            if (roomIter == rooms->end()) {
                roomFound = false;
                cout << "roomfound false" << endl;
            } else {
                roomFound = true;
                cout << "roomfound true" << endl;
            }
            //roomFound = rooms->count(nameString);
            cout << "roomFound = " << roomFound << endl;
        
            //do what is asked by the command
            if (*commandEnum == CREATE) {
                if (roomFound) {
                    answer.status = FAILURE_ALREADY_EXISTS;
                } else {
                    // we create the room
                    int newPort = portFinder->newPort();
                    Room newRoom(name, newPort);
                    rooms->insert({nameString, newRoom});
                    
                
                    //now we pass it to a thread to open the room
                   thread t = thread(roomThread, newPort, portFinder);
                   t.detach();
                   answer.status = SUCCESS;
                }
        
            // } else if (*commandEnum == DELETE) {
        
            // } else if (*commandEnum == JOIN) {
        
            } else {
                answer.status = FAILURE_UNKNOWN;
            }
        }
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

void roomThread(const int port, PortFinder* portFinder) {
    cout << "we opened the room thread" << endl;
}



int acceptRequest(const int sockfd) {
    struct sockaddr_storage their_addr;
	socklen_t sin_size;
	
	sin_size = sizeof their_addr; 
	int client_socket = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
	
	return client_socket;
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