#include "Server.h"

using namespace std;

Server::Server() {
    bzero(&serverAddr,sizeof(serverAddr));
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    serverAddr.sin_port = htons(SERVER_PORT);
    epfd = 0;

    listener = 0;
}

void Server::Init() {
    cout << "start to init server!" << endl;

    listener = socket(PF_INET, SOCK_STREAM, 0);
    if(listener == -1) {
        printf("create socket error: %s(error:%d)\n", strerror(errno), errno);
        exit(0);
    }

    if(bind(listener, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("bind socket error: %s(error:%d)\n", strerror(errno), errno);
        exit(0);
    }

    if(listen(listener, 10) < 0) {
        printf("listen error: %s(error:%d)\n", strerror(errno), errno);
        exit(0);
    }

    cout << "start to listen: " << SERVER_ADDR << endl;

    //创建epoll 句柄epfd
    epfd = epoll_create(EPOLL_SIZE);

    if(epfd < 0) {
        printf("epoll create error: %s(error:%d)\n", strerror(errno), errno);
        exit(0);
    }

    //将listener加入epoll监听
    addfd(epfd, listener, true);
}

void Server::Start() {
    static struct epoll_event events[EPOLL_SIZE];

    Init();

    while(true) {
        int epoll_events_count = epoll_wait(epfd, events, EPOLL_SIZE, 60);
        if(epoll_events_count < 0) {
            printf("epoll wait error: %s(error:%d)\n", strerror(errno), errno);
            exit(0);
        }

        for(int i = 0; i < epoll_events_count; ++i) {
            if(events[i].data.fd == listener) {
                cout << "get a connection" << endl;
                struct sockaddr_in client_address;
                socklen_t client_addrLength = sizeof(struct sockaddr_in);

                int client_fd = accept(listener, (struct sockaddr *)&client_address, &client_addrLength);
                if(client_fd < 0) {
                    printf("epoll wait error: %s(error:%d)\n", strerror(errno), errno);
                    exit(0);
                }
                addfd(epfd, client_fd, true);

                client_list.push_back(client_fd);
                char message[BUFSIZE];
                bzero(message, BUFSIZE);

                sprintf(message, "welcome to connect to server, cliend id: %d\n", client_fd);
                if(send(client_fd, message, BUFSIZE, 0) < 0) {
                    printf("message send error: %s(error:%d)\n", strerror(errno), errno);
                    exit(0);
                }
            } else if(events[i].events & EPOLLIN) {

            }
        }
    }
}

void Server::Stop() {
    close(listener);
    close(epfd);
}
