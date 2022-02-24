#include <pthread.h>
#include <fstream>
#include <string>
#include <map>
#include <deque>

const std::string FILE_PATH = "/db/";
const std::string CLIENT_LIST = FILE_PATH + "clientlist.txt";
const std::string TIMELINE_PATH = FILE_PATH + "tl/";
const std::string TIMELINE_FILE_END = "tl.txt";
const int POST_SIZE = 256;
const int MAX_REPRINT_TIMELINE = 20;

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

		SafeFile* getFile() { return &timelineFile; }
};

std::map<std::string, Client> clientMap;
SafeFile clientList(CLIENT_LIST);

void startupServer() {
	
	clientList.getFile()->lockFile();
	clientList.getFile()->getFileStream().seekg(ios_base::beg);
	while (!clientList.getFileStream().eof()) {
		std::string username;
		username << clientList.getFile()->getFileStream();
		Client addClient(username);
		clientMap.insert({username, addClient});
	}
	clientList.getFile()->unlockFile();
}

class TimelinePrinter {
	private:
		std::deque<std::string> timeline;
		int size;
		Client* client;

		void addToTimeline() {
			if (size == MAX_REPRINT_TIMELINE) {
				timeline.pop_back();
			}

			char[POST_SIZE] buffer;
			client->getFile()->getFileStream().getline(buffer, POST_SIZE);
			std::string post = buffer;

			timeline.push_front(post);
			++size;
		}
	
	public:
		TimelinePrinter(std::string clientName): timeline{}, size{0} {
			Client* client = &clientMap.find(clientName);
			client->getFile()->lockFile();
			client->getFile()->getFileStream().seekg(ios_base::beg);
			while (!client->getFile()->getFileStream().eof()) {
				addToTimeline();
			}
			client->getFile()->unlockFile();
		}

		std::deque<std::string> getTimeline() = { return timeline; }
};