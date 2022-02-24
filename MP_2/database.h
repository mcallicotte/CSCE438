#include <pthread.h>
#include <fstream>
#include <string>
#include <map>

const std::string FILE_PATH = "/db/";
const std::string CLIENT_LIST = FILE_PATH + "clientlist.txt";
const std::string TIMELINE_PATH = FILE_PATH + "tl/";
const std::string TIMELINE_FILE_END = "tl.txt";

class SafeFile {
  private:
    std::string fileName
    pthread_mutex_t fileLock;
    std::fstream fileStream;
    
  public:
    SafeFile(std::string name): fileName{name}, fileLock{PTHREAD_MUTEX_INITIALIZER} { 
        fileStream.open(fileName.c_str(), std::fstream::in | std::fstream::out);
      }
    
    void write(std::string text) {
      pthread_mutex_lock(&fileLock);
      fstream << text;
      pthread_mutex_unlock(&fileLock);
    }
    
    std::fstream getFileStream() { return fileStream; }
    
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
			timelineFile(TIMELINE_PATH + username + TIMELINE_FILE_END);
		}
};

std::map<std::string, Client> clientMap;
SafeFile clientList(CLIENT_LIST);

void startupServer() {
	
	clientList.lockFile();
	clientList.getFileStream().seekg(ios_base::beg);
	while (!clientList.getFileStream().eof()) {
		std::string username;
		username << clientList.getFileStream();
		Client addClient(username);
		clientMap.insert({username, addClient});
	}
	clientList.unlockFile();
}