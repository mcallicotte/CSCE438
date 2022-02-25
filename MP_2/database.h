#include <pthread.h>
#include <fstream>
#include <string>
#include <map>
#include <deque>
#include <iostream>

const std::string FILE_PATH = "db/";
const std::string CLIENT_LIST = FILE_PATH + "clientlist.txt";
const std::string TIMELINE_PATH = FILE_PATH + "tl/";
const std::string TIMELINE_FILE_END = "_tl.txt";
const int POST_SIZE = 256;
const int MAX_REPRINT_TIMELINE = 20;
const int MAX_USERNAME = 32;

class SafeFile {
  private:
    std::string fileName;
    pthread_mutex_t fileLock;
    // std::fstream fileStream;
    
  public:
    std::fstream fileStreamIn;
    std::fstream fileStreamOut;
    SafeFile() { }
    
    SafeFile(std::string name) { 
        //std::cout << "SAFEFILE opening " << fileName << std::endl;
        
        fileName = name;
        fileLock = PTHREAD_MUTEX_INITIALIZER;
        //std::cout << "is it the fileLock?" << std::endl;
        fileStreamOut.open(fileName.c_str(), std::fstream::out | std::fstream::app);
        fileStreamIn.open(fileName.c_str(), std::fstream::in);
        //std::cout << "its the open!" << std::endl;
        
        if (!fileStreamOut.is_open()) {
        	throw std::invalid_argument("File " + fileName + " did not open - out");
        }
        
        if (!fileStreamIn.is_open()) {
        	throw std::invalid_argument("File " + fileName + " did not open - in");
        }
        
        std::cout << "SAFEFILE opened " << fileName << std::endl;
      }
    
    void setFile(std::string name) { 
        fileName = name;
        fileLock = PTHREAD_MUTEX_INITIALIZER;
        fileStreamOut.open(fileName.c_str(), std::fstream::out);
        fileStreamIn.open(fileName.c_str(), std::fstream::in);

		if (!fileStreamOut.is_open()) {
        	throw std::invalid_argument("File " + fileName + " did not open - out");
        }
        
        if (!fileStreamOut.is_open()) {
        	throw std::invalid_argument("File " + fileName + " did not open - in");
        }
      }
    
    void write(std::string text) {
      std::cout << "    start write - " << text << std::endl;
      pthread_mutex_lock(&fileLock);
      fileStreamOut << text;
      pthread_mutex_unlock(&fileLock);
      std::cout << "    end write" << std::endl;
    }
    
    // std::fstream getFileStream() { return fileStream; }
    
    void lockFile() {
    	pthread_mutex_lock(&fileLock);
    }
    
    void unlockFile() {
    	pthread_mutex_unlock(&fileLock);
    }
};

class Client {
	private:
		std::string username;
		SafeFile timelineFile;
		
	public:
		Client(std::string name) {
			//std::cout << "    welcome to client constructor" << std::endl;
			username = name;
			std::string fileName = TIMELINE_PATH + username + TIMELINE_FILE_END;
			std::cout << "    try to create safefile" << std::endl;
			timelineFile = SafeFile(fileName);
			//std::cout << "    leaving client constructor" << std::endl;
		}

		SafeFile* getFile() { return &timelineFile; }
};

std::map<std::string, Client*> clientMap;
SafeFile clientList(CLIENT_LIST);


void printClientMap() {
	//std::cout << "start printmap" << std::endl;
	for (auto i = clientMap.begin(); i != clientMap.end(); i++) {
		std::cout << i->first << " - ";
	}
	std::cout << "end of map" << std::endl;
}

void startupServer() {
	std::cout << "startupServer" << std::endl;
	
	clientList.lockFile();
	clientList.fileStreamIn.seekg(std::ios_base::beg);
	while (!clientList.fileStreamIn.eof()) {
		char buffer[MAX_USERNAME];
		clientList.fileStreamIn.getline(buffer, MAX_USERNAME);
		std::string username = buffer;
		if (username == "" || username == "\n") {
			continue;
		}
		Client* addClient = new Client(username);
		clientMap.insert(std::pair<std::string, Client*>(username, addClient));
	}
	clientList.unlockFile();
	
	printClientMap();
	std::cout << "end startupServer" << std::endl;
}

void cleanServer() {
	//clean the dynamic memory in the clientList
	for (auto i = clientMap.begin(); i != clientMap.end(); i++) {
		delete i->second;
	}
}

class TimelinePrinter {
	private:
		int size;
		Client* client;

		void addToTimeline() {
			if (size == MAX_REPRINT_TIMELINE) {
				timeline.pop_back();
			}

			char buffer[POST_SIZE];
			client->getFile()->fileStreamIn.getline(buffer, POST_SIZE);
			std::string post = buffer;

			timeline.push_front(post);
			++size;
		}
	
	public:
		std::deque<std::string> timeline;
		
		TimelinePrinter(std::string clientName): timeline{}, size{0} {
			client = clientMap.find(clientName)->second;
			client->getFile()->lockFile();
			client->getFile()->fileStreamIn.seekg(std::ios_base::beg);
			while (!client->getFile()->fileStreamIn.eof()) {
				addToTimeline();
			}
			client->getFile()->unlockFile();
		}

};