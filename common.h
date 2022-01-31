#ifndef COMMON_H_
#define COMMON_H_

#include "interface.h"

enum Command {
    CREATE,
    DELETE,
    JOIN,
    LIST,
    UNKNOWN
};

// enum Status
// {
//     SUCCESS,
//     FAILURE_ALREADY_EXISTS,
//     FAILURE_NOT_EXISTS,
//     FAILURE_INVALID,
//     FAILURE_UNKNOWN
// };

struct Answer {
    enum Status status;
    int portNumber;
    int numMembers;
};

struct Chat {
    int clientNumber;
    
    Chat(int number) {
        clientNumber = number;
    }
};

#endif // COMMON_H_