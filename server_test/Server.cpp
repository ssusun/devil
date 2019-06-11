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

                //获取连接套接字
                int client_fd = accept(listener, (struct sockaddr *)&client_address, &client_addrLength);
                if(client_fd < 0) {
                    printf("epoll wait error: %s(error:%d)\n", strerror(errno), errno);
                    exit(0);
                }
                addfd(epfd, client_fd, true);

                client_list.push_back(client_fd);
                //两种写入buf的方式
                // char message[BUFSIZE];
                // bzero(message, BUFSIZE);
                // sprintf(message, "welcome to connect to server, cliend id: %d\n", client_fd);
                string message =  "welcome to connect to server, cliend id: " + to_string(client_fd) + "\n";
                int ret = send(client_fd, message.c_str(), message.length(), 0); 
                // printf("write %d bytes\n", ret);
                if(ret < 0) {
                    printf("message send error: %s(error:%d)\n", strerror(errno), errno);
                    exit(0);
                }

            } else if(events[i].events & EPOLLIN) {
                //收到信息并返回打印
                char buffer[BUFSIZE];
                bzero(buffer, BUFSIZE);
                int read_bytes = read(events[i].data.fd, buffer, BUFSIZE);
                struct Msg msg;
                memset(&msg, 0, sizeof(msg));
                memcpy(&msg, buffer, sizeof(msg));
                printf("read: %d bytes, get message: %s\n", read_bytes, msg.content);
                //通过epoll out事件发送接受信息给客户端
                
                struct epoll_event ev;
                ev.data.fd = events[i].data.fd;
                ev.events = EPOLLOUT | EPOLLET;
                epoll_ctl(epfd, EPOLL_CTL_MOD, events[i].data.fd, &ev);

            } else if(events[i].events & EPOLLOUT) {
                printf("get out event\n");

                char sendBuffer[BUFSIZE];
                struct Msg msg;
                
                memset(&msg, 0, sizeof(msg));
                msg.fromID = 0;
                msg.toID = 0;
                msg.type = 0;
                sprintf(msg.content,"get your message");
                bzero(sendBuffer, BUFSIZE);
                memcpy(sendBuffer, &msg, sizeof(msg));
                
                if(send(events[i].data.fd, sendBuffer, sizeof(sendBuffer), 0) < 0) {
                    printf("message send error: %s(error:%d)\n", strerror(errno), errno);
                    exit(0);
                }
            }
        }
    }
}

void Server::Stop() {
    close(listener);
    close(epfd);
}
