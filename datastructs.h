#ifndef DATASTRUCTS_H_
#define DATASTRUCTS_H_

#include <forward_list>
#include <algorithm>

using namespace std;

struct PortFinder {
    int nextPort;
    int firstPort;
    forward_list<int> reopenedPorts;
    
    PortFinder(const int currentPort) {
        firstPort = currentPort;
        if (firstPort == 1201) {
            nextPort = 1202;
        } else {
            nextPort = 1201;
        }
    }
    
    int newPort() {
        if (reopenedPorts.empty()) {
            nextPort += 1;
            return nextPort;
        } else {
            int port = reopenedPorts.front();
            reopenedPorts.pop_front();
            return port;
        }
    }
    
    void closePort(int port) {
        reopenedPorts.remove(port);
    }
};

struct Room {
    const char* name;
    int mainPort;
    int numMembers;
    forward_list<int> clientPorts;
    
    Room(const char* roomName, const int port) {
        name = roomName;
        mainPort = port;
        numMembers = 0;
    }
    
    void addClient(const int port) {
        clientPorts.push_front(port);
        numMembers++;
    }
    
    void deleteClient(const int port) {
        clientPorts.remove(port);
        numMembers--;
    }
};

#endif