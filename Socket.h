#include <stdlib.h>
#include <stdio.h>

#ifndef SOCKET_H
#define SOCKET_H

#define BACKLOG 199
#define THREADING
#define DEBUGSOCK
#define SOCKBUG

class Socket {
                int *socket_;
        public:
                Socket(int domain, int type, int protocol);
		Socket();
                ~Socket();
                void bind();
                void bind(uint16_t port);
                void listen();
                void listen(int backlog);
                void accept(void *(*start_routine)(void*));
                void accept();
		static void send(int socket, std::string message);
		static void sendFile(int socket, std::string file);
};

#endif

