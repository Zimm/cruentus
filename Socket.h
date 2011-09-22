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
                bool bind();
                bool bind(uint16_t port);
		bool bind(char *path);
		bool connect(char *path);
                bool connect(char *url, uint16_t port);
		bool listen();
                bool listen(int backlog);
                void accept(void *(*start_routine)(void*));
                void accept();
		void send(std::string message);
		bool sendFile(std::string file);
		void close();
};

void server_setEcho(bool e);

#endif

