#ifndef DATASTRUCTS_H_
#define DATASTRUCTS_H_

#include <forward_list>
#include <algorithm>
#include <mutex>

using namespace std;

struct PortFinder {
    int nextPort;
    int firstPort;
    forward_list<int> reopenedPorts;
    mutex pfLock;
    
    PortFinder(const int currentPort) {
        firstPort = currentPort;
        if (firstPort == 1201) {
            nextPort = 1202;
        } else {
            nextPort = 1201;
        }
    }
    
    int newPort() {
        int port;
        unique_lock<mutex> lock(pfLock);
        
        if (reopenedPorts.empty()) {
            port = nextPort;
            nextPort++;
        } else {
            int port = reopenedPorts.front();
            reopenedPorts.pop_front();
        }
        
        return port;
    }
    
    void closePort(int port) {
        unique_lock<mutex> lock(pfLock);
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