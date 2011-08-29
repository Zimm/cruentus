#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/un.h>
#ifndef __APPLE__
#include <sys/sendfile.h>
#endif
#include "Socket.h"

using namespace std;

static bool echo_server = false;

void server_setEcho(bool e) {
	echo_server = e;
}

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

Socket::Socket(int fd) {
	socket_ = new int;
	*socket_ = fd;
}


Socket::Socket(int domain, int type, int protocol) {

	socket_ = new int;

        int sock_descriptor = socket(domain, type, protocol);
        if (sock_descriptor == -1) {
                fprintf(stderr, "Failed to open custom socket\n");
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

void Socket::bind(char *path) {
	
	struct sockaddr_un address;
	address.sun_family = AF_UNIX;
	strcpy(address.sun_path, path);
	unlink(address.sun_path);
	size_t len = strlen(address.sun_path) + sizeof(address.sun_family);
	if (::bind(*socket_,(struct sockaddr *)&address,len) == -1) {
		fprintf(stderr, "Failed to bind unix\n");
		return;
	}
#ifdef DEBUG
        cout << "Binded to file " << address.sun_path << endl;
#endif
}

void Socket::connect(char *path) {
	
	struct sockaddr_un remote;
	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, path);
	size_t len = strlen(remote.sun_path) + sizeof(remote.sun_family);
	if (::connect(*socket_, (struct sockaddr *)&remote, len) == -1) {
        	fprintf(stderr, "Failed to connect\n");
        	return;
    	}
#ifdef DEBUG
	cout << "Connected to file " << remote.sun_path << endl;
#endif
}

void Socket::bind(uint16_t port) {
	
	struct sockaddr_in address;

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	

	if (::bind(*socket_,(struct sockaddr *)&address,sizeof(address)) == -1) {

		fprintf(stderr, "Failed to bind socket\n");
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
                fprintf(stderr, "Failed to listen socket\n");
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
		if (echo_server)
			send(sock, buffer, bufsize, 0);
	}
	return NULL;
}
	

void Socket::accept() {
	accept(tester);
}

void Socket::send(std::string message) {
#ifdef DEBUG	
	cout<<"Sending " << message.c_str() << endl;
#endif
	::send(*socket_, message.c_str(), message.length(), 0);

}

void Socket::sendFile(std::string file) {
#ifdef DEBUG
	cout << "Sending file " << file << endl;
#endif
	int socket = *socket_;
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
	sendfile(socket, filed, NULL, st.st_size);
	leng = st.st_size;
#endif
#ifdef DEBUG
	cout << "Sent " << leng << " bytes" << endl;
#endif
	close(filed);
}

