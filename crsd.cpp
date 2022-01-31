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
#include <vector>
#include <fcntl.h>


using namespace std;

int startServer(const int portNumber);
int acceptRequest(const int sockfd);
void processCommand(const int clientSocket, PortFinder* portFinder, map<string, Room>* rooms);
void roomMainThread(const int port, PortFinder* portFinder);
void roomClientThread(const int clientSocket, const vector<int>* clientSockets, vector<thread>* threads);
void roomBroadcastThread(char* chatMessage, const vector<int>* clientSockets);
int connect_to(const char *host, const int port);

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
                   thread t = thread(roomMainThread, newPort, portFinder);
                   t.detach();
                   answer.status = SUCCESS;
                }
        
             } else if (*commandEnum == DELETE) {
                 if (!roomFound) {
                     answer.status = FAILURE_NOT_EXISTS;
                 } else {
                    //send a message to the room that it is time to close
                    // we set up quit message and connect to main room port
                    Chat stop(-2);
                    char message[sizeof(Chat) + 1048];
                    memcpy(message, &stop, sizeof(Chat));
                    strcpy(message + sizeof(Chat), "SERVER: DELETING ROOM");
                    int newSocket = connect_to("127.0.0.1", roomIter->second.mainPort);
                    
                    //send message to main room that it is time to close 
                    send(newSocket, message, sizeof(Chat), 0);
                    
                    //remove the room from the rooms list
                    rooms->erase(roomIter);
                    
                    answer.status = SUCCESS;
                 }
        
            } else if (*commandEnum == JOIN) {
                if (!roomFound) {
                    answer.status = FAILURE_NOT_EXISTS;
                } else {
                    int masterPort = roomIter->second.mainPort;
                }
        
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

void roomMainThread(const int port, PortFinder* portFinder) {
    cout << "we opened the room thread" << endl;
    vector<int> clientSockets;
    vector<thread> clientThreads;
    
    int sockfd = startServer(port);
	
	while(true) {
		int client_socket = acceptRequest(sockfd);
		if (client_socket == -1) {
		    perror("accept");
		    continue;
		}
		cout << "accepted a request - Room " << port << endl;
		
		char request[sizeof(Chat) + 1048];
		recv(client_socket, request, sizeof(Chat) + 1048, 0);
		
		Chat* chat = (Chat*) request;
		if (chat->clientNumber == -2) {
		    //we would delete the room
		    //broadcast the warning message and make sure it goes through
		    thread t = thread(roomBroadcastThread, request, &clientSockets);
		    t.join();
		    
		    // then we can destroy all child threads
		    for(int i = 0; i < clientThreads.size(); i++) {
		        clientThreads[i].~thread();
		    }
		    
		    //we end this thread by just ending its execution
		    return;
		} else if (chat->clientNumber == -1) {
		    //we will respond with the clientNumber and add to vector
		    clientSockets.push_back(client_socket);
		    Chat firstChat(clientSockets.size() - 1);
		    clientThreads.push_back(thread(roomClientThread, client_socket, &clientSockets, &clientThreads));
		    clientThreads.back().detach();
		} else {
		    // we have to broadcast a message
		}
	}
}

void roomClientThread(const int clientSocket, const vector<int>* clientSockets, vector<thread>* threads) {
    while(true) {
        char request[sizeof(Chat) + 1048];
        recv(clientSocket, request, sizeof(Chat) + 1048, 0);
        threads->push_back(thread(roomBroadcastThread, request, clientSockets));
        threads->back().detach();
    }
}

void roomBroadcastThread(char* chatMessage, const vector<int>* clientSockets) {
    Chat* chat = (Chat*) chatMessage;
    
    char message[1048];
    memcpy(message, chatMessage + sizeof(Chat), 1048);
    message[1047] = '\0';
    
    for(int i = 0; i < clientSockets->size(); i++) {
        if (i != chat->clientNumber) {
            send(clientSockets->at(i), message, 1048, 0);
        }
    }
    
}

int connect_to(const char *host, const int port)
{
	// ------------------------------------------------------------
	// GUIDE :
	// In this function, you are suppose to connect to the server.
	// After connection is established, you are ready to send or
	// receive the message to/from the server.
	// 
	// Finally, you should return the socket fildescriptor
	// so that other functions such as "process_command" can use it
	// ------------------------------------------------------------

    // below is just dummy code for compilation, remove it.
	// int sockfd = -1;
	// return sockfd;
	
	struct addrinfo hints;
	struct addrinfo *res;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = 0;
    int status;
    
    char portReady[33];
    sprintf(portReady,"%d",port);

    if ((status = getaddrinfo(host, portReady, &hints, &res)) != 0) {
        cerr << "getaddrinfo: " << gai_strerror(status) << endl;
    }

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        perror("Cannot create socket");
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
       perror("Cannot connect");
    }
    
    return sockfd;
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