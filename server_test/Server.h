#ifndef SERVER_H
#define SERVER_H

#include "Common.h"
using namespace std;

class Server {
public:
    Server();

    void Init();
    void Start();
    void Stop();

private:
    struct sockaddr_in serverAddr;
    int listener;
    int epfd;
    list<int> client_list;
};
#endif
