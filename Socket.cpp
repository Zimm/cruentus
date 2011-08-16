#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <arpa/inet.h>
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

	cout << "Binded to port " << ntohs(address.sin_port) << endl;

}


void Socket::listen() {
	listen(1);
}

void Socket::listen(int backlog) {
	if (::listen(*socket_, backlog) == -1) {
                fprintf(stderr, "Failed to open socket\n");
                return;
        }
}


void Socket::accept(void *(*start_routine)(void*)) {
	while (1) {
		struct sockaddr_in inAddress;
		int socklen = sizeof(sockaddr_in);
		cout << "Waiting for a connection...." << endl;
		int openedSocket = ::accept(*socket_, (struct sockaddr *)&inAddress, (socklen_t *)&socklen);
		cout << "Connected!" << endl;
		if (openedSocket < 0) {
			fprintf(stderr, "Failed to accept incoming socket\n");
			return;
		}
		pthread_t newThread;
		cout << "Creating new thread for socket "<< openedSocket << endl;
		int rc = pthread_create(&newThread, NULL, start_routine, (void*)&openedSocket);
		cout << "Created new thread " << rc << endl;
		if (rc){
         		fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}
}

extern "C" void *tester(void *socket) {

	int sock = *((int*)socket);
	cout << "Created this thread with sock " << sock << endl;
	while (1) {
		int bufsize=1024;
		char *buffer = (char *)calloc(1,bufsize);
		recv(sock,buffer,bufsize,0);
		
		cout << buffer << endl;
	}
}
	

void Socket::accept() {
	accept(tester);
}

