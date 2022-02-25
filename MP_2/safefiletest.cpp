#include "database.h"

using namespace std;

int main() {
    const string FILEPATH = "filetest/";
    const string testExisting = FILEPATH + "readTest.txt";
    const string testNew = FILEPATH + "newTest.txt";

    SafeFile existing(testExisting);
    SafeFile newFile(testNew);

    existing.write("write1\n");

    newFile.write("write2\n");

    char buffer[POST_SIZE];
    existing.fileStreamIn.getline(buffer, POST_SIZE);
    cout << "first read of existing: " << buffer << endl;

    
    return 1;
}