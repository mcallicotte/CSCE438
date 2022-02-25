#include <pthread.h>
#include <fstream>
#include <string>
#include <map>
#include <deque>
#include <iostream>

const std::string FILE_PATH = "db/";
const std::string CLIENT_LIST = FILE_PATH + "clientlist.txt";
const std::string TIMELINE_PATH = FILE_PATH + "tl/";
const std::string TIMELINE_FILE_END = "tl.txt";
const int POST_SIZE = 256;
const int MAX_REPRINT_TIMELINE = 20;
const int MAX_USERNAME = 32;

class SafeFile {
  private:
    std::string fileName;
    pthread_mutex_t fileLock;
    // std::fstream fileStream;
    
  public:
    std::fstream fileStream;
    
    SafeFile() { }
    
    SafeFile(std::string name) { 
        fileName = name;
        fileLock = PTHREAD_MUTEX_INITIALIZER;
        fileStream.open(fileName.c_str(), std::fstream::in | std::fstream::out);
        
        if (!fileStream.is_open()) {
        	throw std::invalid_argument("File did not open");
        }
      }
    
    void setFile(std::string name) { 
        fileName = name;
        fileLock = PTHREAD_MUTEX_INITIALIZER;
        fileStream.open(fileName.c_str(), std::fstream::in | std::fstream::out);
      }
    
    void write(std::string text) {
      pthread_mutex_lock(&fileLock);
      fileStream << text;
      pthread_mutex_unlock(&fileLock);
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
			username = name;
			std::string fileName = TIMELINE_PATH + username + TIMELINE_FILE_END;
			timelineFile.setFile(fileName);
		}

		SafeFile* getFile() { return &timelineFile; }
};

std::map<std::string, Client*> clientMap;
SafeFile clientList(CLIENT_LIST);

void startupServer() {
	std::cout << "startupServer" << std::endl;
	clientList.lockFile();
	clientList.fileStream.seekg(std::ios_base::beg);
	while (!clientList.fileStream.eof()) {
		char buffer[MAX_USERNAME];
		clientList.fileStream.getline(buffer, MAX_USERNAME);
		std::string username = buffer;
		Client addClient(username);
		clientMap.insert(std::pair<std::string, Client*>(std::string(buffer), new Client(username)));
	}
	clientList.unlockFile();
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
			client->getFile()->fileStream.getline(buffer, POST_SIZE);
			std::string post = buffer;

			timeline.push_front(post);
			++size;
		}
	
	public:
		std::deque<std::string> timeline;
		
		TimelinePrinter(std::string clientName): timeline{}, size{0} {
			client = clientMap.find(clientName)->second;
			client->getFile()->lockFile();
			client->getFile()->fileStream.seekg(std::ios_base::beg);
			while (!client->getFile()->fileStream.eof()) {
				addToTimeline();
			}
			client->getFile()->unlockFile();
		}

};