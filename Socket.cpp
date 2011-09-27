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
#include <errno.h>
#include <netdb.h>
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
		errno = 41;
	}

	int on = 1;
	if (setsockopt(sock_descriptor, SOL_SOCKET, SO_REUSEADDR, (const char*) &on, sizeof(on)) == -1) {
		fprintf(stderr, "Failed to set sock options\n");
		errno = 42;
	}

	*socket_ = sock_descriptor;
	errno = 0;
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
                errno = 43;
        }

        *socket_ = sock_descriptor;
	errno = 0;
}

Socket::~Socket() {
	delete socket_;
}

void Socket::close() {
	::close(*socket_);
}

bool Socket::bind() {
	
	return bind(7331);

}

bool Socket::bind(char *path) {
	
	struct sockaddr_un address;
	address.sun_family = AF_UNIX;
	strcpy(address.sun_path, path);
	unlink(address.sun_path);
	size_t len = strlen(address.sun_path) + sizeof(address.sun_family);
	if (::bind(*socket_,(struct sockaddr *)&address,len) == -1) {
		fprintf(stderr, "Failed to bind unix\n");
		return false;
	}
#ifdef DEBUG
        cout << "Binded to file " << address.sun_path << endl;
#endif
	return true;
}

bool Socket::connect(char *path) {
	
	struct sockaddr_un remote;
	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, path);
	size_t len = strlen(remote.sun_path) + sizeof(remote.sun_family);
	if (::connect(*socket_, (struct sockaddr *)&remote, len) == -1) {
        	fprintf(stderr, "Failed to connect %i\n", errno);
        	return false;
    	}
#ifdef DEBUG
	cout << "Connected to file " << remote.sun_path << endl;
#endif
	return true;
}

bool Socket::connect(char *url, uint16_t port) {
	
	struct sockaddr_in serv_addr;
	struct hostent *server;
	server = gethostbyname(url);
	if (server == NULL) {
		fprintf(stderr, "Failed to get host %i\n", errno);
		return false;
	}
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(port);
	if (::connect(*socket_,(const struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
		fprintf(stderr, "Failed to connect %i\n", errno);
		return false;
	}
#ifdef DEBUG
	cout << "Connected to " << url << ":" << port << endl;
#endif
	return true;
}


bool Socket::bind(uint16_t port) {
	
	struct sockaddr_in address;

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	

	if (::bind(*socket_,(struct sockaddr *)&address,sizeof(address)) == -1) {

		fprintf(stderr, "Failed to bind socket\n");
                return false;

	}
#ifdef DEBUG
	cout << "Binded to port " << ntohs(address.sin_port) << endl;
#endif
	return true;
}


bool Socket::listen() {
	return listen(1);
}

bool Socket::listen(int backlog) {
	if (::listen(*socket_, backlog) == -1) {
                fprintf(stderr, "Failed to listen socket\n");
        	return false;
	}
	return true;
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
			continue;
		}
#ifdef TESTSOCK
		if (openedSocket != lastSocket) {
			::close(lastSocket);
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
				//exit(-1);
				::close(openedSocket);
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

bool Socket::sendFile(std::string file) {
#ifdef DEBUG
	cout << "Sending file " << file << endl;
#endif
	int socket = *socket_;
	int filed = open(file.c_str(), O_RDONLY);
	if (filed == -1) {
		cout << "Failed to open " << file << " for reading" << endl;
		return false;
	}
	off_t leng;
#ifdef __APPLE__
	sendfile(socket, filed, 0, &leng, NULL, 0);
#else
	struct stat st;
	if (stat(file.c_str(), &st) != 0) {
		cout << "No file found at " << file << endl;
		::close(filed);
		return false;
	}
	sendfile(socket, filed, NULL, st.st_size);
	leng = st.st_size;
#endif
#ifdef DEBUG
	cout << "Sent " << leng << " bytes" << endl;
#endif
	::close(filed);
	return true;
}

