#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <iostream>

#ifndef SOCKET_H
#define SOCKET_H

#define BACKLOG 10
#define THREADING

class Socket {
	public:
                int *socket_;
                Socket(int domain, int type, int protocol);
		Socket();
		Socket(int fd);
                ~Socket();
                void bind();
                void bind(uint16_t port);
		void bind(char *path);
		void connect(char *path);
                void listen();
                void listen(int backlog);
                void accept(void *(*start_routine)(void*));
                void accept();
		void send(std::string message);
		void sendFile(std::string file);
};

void server_setEcho(bool e);

#endif

