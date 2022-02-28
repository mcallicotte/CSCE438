#include <pthread.h>
#include <fstream>
#include <string>
#include <map>
#include <deque>
#include <iostream>

#include <chrono>
#include <ctime>
#include <signal.h>
#include <unistd.h>

#include <google/protobuf/timestamp.pb.h>
#include <google/protobuf/duration.pb.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <google/protobuf/util/time_util.h>
#include <grpc++/grpc++.h>

#include "sns.grpc.pb.h"
#include "sns.pb.h"

const std::string FILE_PATH = "db/";
const std::string CLIENT_LIST = FILE_PATH + "clientlist.txt";
const std::string TIMELINE_PATH = FILE_PATH + "tl/";
const std::string TIMELINE_FILE_END = "_tl.txt";
const int POST_SIZE = 256;
const int MAX_REPRINT_TIMELINE = 20;
const int MAX_USERNAME = 32;
const std::string FOLLOWERS_PATH = FILE_PATH + "fl/";
const std::string FOLLOWERS_FILE_END = "_fl.txt";

class SafeFile {
  private:
    std::string fileName;
    pthread_mutex_t fileLock;
    // std::fstream fileStream;
    
  public:
    std::fstream fileStreamIn;
    std::fstream fileStreamOut;
    SafeFile() { }
    
    SafeFile(std::string name, bool truncate = false) { 
        //std::cout << "SAFEFILE opening " << fileName << std::endl;
        
        fileName = name;
        fileLock = PTHREAD_MUTEX_INITIALIZER;
        //std::cout << "is it the fileLock?" << std::endl;
        if (truncate) {
        	fileStreamOut.open(fileName.c_str(), std::fstream::out | std::fstream::trunc);
        } else {
        	fileStreamOut.open(fileName.c_str(), std::fstream::out | std::fstream::app);
        }
        
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
      //std::cout << "    start write - " << text << std::endl;
      pthread_mutex_lock(&fileLock);
      fileStreamOut << text;
      pthread_mutex_unlock(&fileLock);
      //std::cout << "    end write" << std::endl;
    }
    
    // std::fstream getFileStream() { return fileStream; }
    
    void lockFile() {
    	pthread_mutex_lock(&fileLock);
    }
    
    void unlockFile() {
    	pthread_mutex_unlock(&fileLock);
    }
    
    void closeStreams() {
    	fileStreamIn.close();
    	fileStreamOut.close();
    }
    
    void outputFile() {
    	lockFile();
    	
    }
};

class Post {
	private:
		 std::string username;
		 const std::string timestamp = "TIME";
		 std::string text;
		 
	public:
		std::string getUsername() { return username; }
		std::string getTimestamp() { return timestamp; }
		std::string getText() { return text; }
		
		void setUsername(std::string name) { username = name; }
		// void setTimestamp(std::string stamp) { timestamp = stamp; }
		// void setTimestamp() {
		// 	time_t currentTime; 
  //      	timestamp = std::to_string(time(&currentTime));
		// }
		void setText(std::string message) { text = message; }
		
		std::string printPost() {
			return username + "(" + timestamp + ") >>" + text;
		}
};

class Timeline {
	private:
		int size;
		SafeFile* timelineFile;
		int messageIncrement;
	
	public:
		std::deque<Post> timeline;

		void addToTimeline() {
			char buffer1[POST_SIZE];
			char buffer2[POST_SIZE];
			char buffer3[POST_SIZE];
			Post post;
			
			if (size == MAX_REPRINT_TIMELINE) {
				timeline.pop_back();
			}
				
			timelineFile->fileStreamIn.getline(buffer1, POST_SIZE);
			post.setUsername(std::string(buffer1));
			
		
			timelineFile->fileStreamIn.getline(buffer2, POST_SIZE);
			//post.setTimestamp(std::string(buffer2));
			
			timelineFile->fileStreamIn.getline(buffer3, POST_SIZE);
			post.setText(std::string(buffer3));

			if (post.getUsername() != "") {
				timeline.push_front(post);
				++size;
				messageIncrement++;
				std::cout << "added " << post.printPost() << "to timeline" << std::endl;
			}
		}

		void pushTimeline(Post post) {
			if (size == MAX_REPRINT_TIMELINE) {
				timeline.pop_back();
			}

			timeline.push_front(post);
			++size;
			std::cout << "added \"" << post.printPost() << "\" to timeline" << std::endl;
			messageIncrement++;
		}

		Timeline() { }
		
		void printTimeline() {
			for (auto i = timeline.begin(); i != timeline.end(); i++) {
				std::cout << i->printPost() << std::endl;
			}
		}

		int getMessageIncrement() { return messageIncrement; }
		int getSize() { return size; }

		Timeline(std::string clientName, SafeFile* file): timeline{}, size{0}, messageIncrement{0} {
			//client = clientMap.find(clientName)->second;
			timelineFile = file;
			timelineFile->lockFile();
			timelineFile->fileStreamIn.seekg(std::ios_base::beg);
			while (!timelineFile->fileStreamIn.eof()) {
				addToTimeline();
			}
			timelineFile->unlockFile();
		}

};



class Client {
	
	private:
		std::string username;
		SafeFile timelineFile;
		SafeFile followerFile;
		int followerCount = 0;
		grpc::ServerReaderWriter<csce438::Message, csce438::Message>* stream;
		bool hasStream = false;
	public:
		SafeFile* getTimelineFile() { return &timelineFile; }
		SafeFile* getFollowerFile() { return &followerFile; }
		std::string getUsername() { return username; }
	private:
		
		Timeline timeline;
		//int timelinesOpen = 0;
		
	public:
		std::map<std::string, int> followerMap;
		Client(std::string name): followerMap{} {
			//std::cout << "    welcome to client constructor" << std::endl;
			username = name;
			std::string fileName = TIMELINE_PATH + username + TIMELINE_FILE_END;
			std::cout << "    try to create safefile" << std::endl;
			timelineFile = SafeFile(fileName);
			//std::cout << "    leaving client constructor" << std::endl;
			fileName = FOLLOWERS_PATH + username + FOLLOWERS_FILE_END;
			followerFile = SafeFile(fileName);

			//start the follower map
			followerFile.lockFile();
			followerFile.fileStreamIn.seekg(std::ios_base::beg);
			int lineCount = 1;
			while (!followerFile.fileStreamIn.eof()) {
				char buffer[MAX_USERNAME];
				followerFile.fileStreamIn.getline(buffer, POST_SIZE);
				std::string fname = buffer;
				followerMap.insert({fname, lineCount});
				++lineCount;
			}
			followerFile.unlockFile();

			followerCount = followerMap.size();

			timeline = Timeline(username, &timelineFile);
		}
		
		Timeline getTimeline() { return timeline; }
		
		//void openTimeline() { timelinesOpen++; };
		//int getTimelinesOpen() { return timelinesOpen; }
		
		~Client() {
			timelineFile.closeStreams();
			followerFile.closeStreams();

			followerFile = SafeFile(FOLLOWERS_PATH + username + FOLLOWERS_FILE_END, 1);
			
			for (auto i = followerMap.begin(); i != followerMap.end(); i++) {
				followerFile.write(i->first + "\n");
			}
			
			// timelineFile = SafeFile(TIMELINE_PATH + username + TIMELINE_FILE_END, 1);
			
			// for (auto i = getTimeline().timeline.rbegin(); i != getTimeline().timeline.rend(); i++) {
			// 	auto element = i--;
				
			// 	timelineFile.write(element->getUsername() + "\n");
			// 	timelineFile.write(element->getTimestamp() + "\n");
			// 	timelineFile.write(element->getText() + "\n");
			// }
			
			timelineFile.closeStreams();
			followerFile.closeStreams();
		}

		bool addFollower(std::string fname) {
			if (followerMap.find(fname) == followerMap.end()) {
				followerCount++;
				followerMap.insert({fname, followerCount});
				return true;
			} else {
				//already following
				return false;
			}
		}

		bool removeFollower(std::string fname) {
			auto followerIter = followerMap.find(fname);
			if (followerIter != followerMap.end()) {
				followerMap.erase(followerIter);
				followerCount--;
				return true;
			} else {
				// it is end, so this name isn't following
				return false;
			}
		}
		
		void addStream(grpc::ServerReaderWriter<csce438::Message, csce438::Message>* theStream) {
			stream = theStream;
			hasStream = true;
		}
		
		bool getStreamStatus() {
			return hasStream;
		}
		
		grpc::ServerReaderWriter<csce438::Message, csce438::Message>* getStream() { return stream; }
};

std::map<std::string, Client*> clientMap;
//SafeFile clientList;


void printClientMap() {
	//std::cout << "start printmap" << std::endl;
	for (auto i = clientMap.begin(); i != clientMap.end(); i++) {
		std::cout << i->first << " - ";
	}
	std::cout << "end of map" << std::endl;
}

void startupServer() {
	std::cout << "startupServer" << std::endl;
	
	SafeFile clientList = SafeFile(CLIENT_LIST);
	
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
	clientList.closeStreams();
	
	printClientMap();
	std::cout << "end startupServer" << std::endl;
}

void cleanServer() {
	//clean the dynamic memory in the clientList
	SafeFile clientList(CLIENT_LIST, 1);
	
	for (auto i = clientMap.begin(); i != clientMap.end(); i++) {
		clientList.write(i->second->getUsername() + "\n");
		delete i->second;
	}
	clientList.closeStreams();
}

