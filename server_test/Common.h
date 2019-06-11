#ifndef COMMON_H
#define COMMON_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <list>
#include <iostream>

#define BUFSIZE 0xFFFF

#define SERVER_ADDR "127.0.0.1"

#define SERVER_PORT 10200

#define EPOLL_SIZE 5000


static void addfd(int epfd, int listener, bool isET) {
    //将socket设置为非阻塞
    int flag = fcntl(listener, F_GETFL, 0);
    fcntl(listener, F_SETFL, flag | O_NONBLOCK);

    //将listener加入epoll监听
    struct epoll_event ev;
    ev.data.fd = listener;
    ev.events = EPOLLIN;
    if(isET)
        ev.events |= EPOLLET;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listener, &ev);
    std::cout << "listener added to epoll" << std::endl;
}

struct Msg {
    int type;
    int fromID;
    int toID;
    char content[BUFSIZE];
};
#endif
