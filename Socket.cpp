#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/uio.h>
#ifndef __APPLE__
#include <sys/sendfile.h>
#endif
#include "Socket.h"

using namespace std;

Socket::Socket() {
	
	socket_ = new int;

	int sock_descriptor = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_descriptor == -1) {
		fprintf(stderr, "Failed to open socket\n");
		exit(-1);
	}

	int on = 1;
	if (setsockopt(sock_descriptor, SOL_SOCKET, SO_REUSEADDR, (const char*) &on, sizeof(on)) == -1) {
		fprintf(stderr, "Failed to set sock options\n");
		exit(-1);
	}

	*socket_ = sock_descriptor;

}

Socket::Socket(int domain, int type, int protocol) {

	socket_ = new int;

        int sock_descriptor = socket(domain, type, protocol);
        if (sock_descriptor == -1) {
                fprintf(stderr, "Failed to open socket\n");
                exit(-1);
        }

        *socket_ = sock_descriptor;

}

Socket::~Socket() {
	close (*socket_);
	delete socket_;
}

void Socket::bind() {
	
	bind(7331);

}

void Socket::bind(uint16_t port) {
	
	struct sockaddr_in address;

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	

	if (::bind(*socket_,(struct sockaddr *)&address,sizeof(address)) == -1) {

		fprintf(stderr, "Failed to open socket\n");
                return;

	}
#ifdef DEBUG
	cout << "Binded to port " << ntohs(address.sin_port) << endl;
#endif
}


void Socket::listen() {
	listen(1);
}

void Socket::listen(int backlog) {
	if (::listen(*socket_, backlog) == -1) {
                fprintf(stderr, "Failed to open socket\n");
                exit(-1);
        }
}

#ifdef TESTSOCK
static int lastSocket = -1;
#endif

void Socket::accept(void *(*start_routine)(void*)) {
	while (1) {
		struct sockaddr_in inAddress;
		int socklen = sizeof(sockaddr_in);
		int openedSocket = ::accept(*socket_, (struct sockaddr *)&inAddress, (socklen_t *)&socklen);
		if (openedSocket < 0) {
			fprintf(stderr, "Failed to accept incoming socket\n");
			exit(-1);
		}
#ifdef TESTSOCK
		if (openedSocket != lastSocket) {
			close(lastSocket);
			lastSocket = openedSocket;
		}
#endif
#ifdef SOCKBUG
		cout << "Got socket " << openedSocket << endl;
#endif
#ifdef PROCS
		if (fork() == 0) {
#endif
#ifdef THREADING
			pthread_t newThread;
			int rc = pthread_create(&newThread, NULL, start_routine, new int(openedSocket));
			if (rc){
         			fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", rc);
				exit(-1);
			}
#else
			start_routine((void*)&openedSocket);
#endif
#ifdef PROCS	
			break;
		}
#endif
	}
}

extern "C" void *tester(void *socket) {

	int sock = *((int*)socket);
	while (1) {
		int bufsize=1024;
		char *buffer = (char *)calloc(1,bufsize);
		recv(sock,buffer,bufsize,0);
		
		cout << buffer << endl;
	}
	return NULL;
}
	

void Socket::accept() {
	accept(tester);
}

void Socket::send(int socket, std::string message) {
#ifdef DEBUG	
	cout<<"Sending " << message.c_str() << endl;
#endif
	::send(socket, message.c_str(), message.length(), 0);

}

void Socket::sendFile(int socket, std::string file) {
#ifdef DEBUG
	cout << "Sending file " << file << endl;
#endif
	int filed = open(file.c_str(), O_RDONLY);
	if (filed == -1) {
		cout << "Failed to open " << file << " for reading" << endl;
		return;
	}
	off_t leng;
#ifdef __APPLE__
	sendfile(filed, socket, 0, &leng, NULL, 0);
#else
	struct stat st;
	if (stat(file.c_str(), &st) != 0) {
		cout << "No file found at " << file << endl;
		close(filed);
		return;
	}
	sendfile(filed, socket, NULL, st.st_size);
	leng = st.st_size;
#endif
#ifdef DEBUG
	cout << "Sent " << leng << " bytes" << endl;
#endif
	close(filed);
}

