#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "Socket.h"
#include "Server.h"

using namespace std;

static bool utility_ = false;

extension extensions[] = {
	{(char *)"j", (char *)"text/javascript"},
        {(char*)"gif", (char*)"image/gif" },
        {(char*)"jpg", (char*)"image/jpeg"},
        {(char*)"jpeg",(char*)"image/jpeg"},
        {(char*)"png", (char*)"image/png" },
        {(char*)"zip", (char*)"image/zip" },
        {(char*)"gz",  (char*)"image/gz"  },
        {(char*)"tar", (char*)"image/tar" },
        {(char*)"htm", (char*)"text/html" },
        {(char*)"html",(char*)"text/html" },
	{(char *)"css", (char *)"text/css"},
	{(char *)"js", (char *)"text/javascript"},
	{(char *)"mp4", (char *)"video/mp4"},
	{(char *)"tiff", (char *)"image/tiff"},
	{(char *)"pdf", (char *)"application/pdf"},
        {0,0} };

void server_setUtility(bool ut) {
	utility_ = ut;
}

void *server(void *socket) {
	int sock = *((int*)socket);
#ifdef DEBUGSOCK
	cout << pthread_self() << ": Got socket " << sock << endl;
#endif
	int bufsize=1024;
	char *buffer = (char *)calloc(1,bufsize);
	recv(sock,buffer,bufsize,0);
#ifdef DEBUG
	cout << "Request:" << endl << buffer << endl<<"Done"<<endl;
#endif
	//get the url they want.....
	if (strncmp(buffer, "GET ", 4) && strncmp(buffer,"get ",4)) {
		Socket::send(sock, string("Sorry only GET is allowed atm"));
		free(buffer);
		return NULL;
	}		
	
	string request(buffer);
	request = request.substr(4);
	size_t pos = request.find(" HTTP/1");
	request = request.substr(0,pos);
	
#ifdef DEBUGFILE
	cout << "Requesting " << request << endl;
#endif
	if (strcmp(request.c_str(), "/") == 0 && !utility_)
		request = "/index.html";
#ifdef DEBUG
	cout << "Trying to get " << request << endl;
#endif 
	struct stat st;

	request.insert(0,string("."));

	if (stat(request.c_str(), &st) != 0 || S_ISDIR(st.st_mode)) {
#ifdef DEBUGFAIL
		cout << "failed to get resource " << request << endl;
#endif
		if (!utility_) {
			Socket::send(sock, string("HTTP/1.1 404 Not Found\r\n"));
			Socket::send(sock, string("Content-Type: text/html; charset=UTF-8\r\n\r\n"));
			Socket::send(sock, string("<html><body>Goodbye World</body></html>"));
		} else {
		
			DIR *dp;
			struct dirent *ep;     
			dp = opendir (request.c_str());
			if (dp != NULL) {
				Socket::send(sock,string("HTTP/1.1 200 OK\r\n"));
				Socket::send(sock, string("Content-Type: text/html; charset=UTF-8\r\n\r\n"));
				Socket::send(sock,string("<html><body>"));
				while ((ep = readdir (dp)) != NULL) {
					Socket::send(sock,string(ep->d_name));
					Socket::send(sock,string("<br/>"));
				}	
				Socket::send(sock, string("</body></html>"));
				closedir (dp);
			} else {
#ifdef DEBUGFAIL
				cout << "Failed to get resource dir" << endl;
#endif
				Socket::send(sock,string("HTTP/1.1 404 Not Found\r\n"));
				Socket::send(sock,string("Content-Type: text/html; charset=UTF-8\r\n\r\n"));
				Socket::send(sock, string("<html><body>Goodbye World</body></html>"));
			}	
		}
	
	} else {
		size_t leng = request.length();
		char *fstr = (char *)0;
		for(int i=0;extensions[i].ext != 0;i++) {
			size_t len = strlen(extensions[i].ext);
			if (!strncmp(request.substr(leng-len).c_str(), extensions[i].ext, len)) {
				fstr =extensions[i].filetype;
				break;
			}
		}
			
		string header("HTTP/1.1 200 OK\r\n");
		
		time_t rawtime;
		time(&rawtime);
		header += "Date: ";
		header += ctime(&rawtime);

		if (fstr == 0) {
#ifdef DEBUG
			cout << "Didn't find a file type.... " << endl;
#endif
			header += "Content-Type: text/plain; charset=UTF-8\r\n";
		} else {
			header += "Content-Type: ";
			header += fstr;
			header += "; charset=UTF-8\r\n";
			if (strcmp(fstr, "text/html") && strcmp(fstr, "text/plain")) {
				header += "Content-Disposition: attachment\r\n";
			}
		}
		header += "Content-Length: ";
		
		ostringstream result;
		result << st.st_size;
		header += result.str();
	
		header += "\r\n\r\n";
#ifdef DEBUGFILE
		cout << "Sending " << header << " for " << request << endl;
#endif
		
		Socket::send(sock, header);
	
		Socket::sendFile(sock, request);
	
	}
#ifdef DEBUGSOCK
	cout << pthread_self() << ": Closing socket" << sock << endl;
#endif
#ifndef TESTSOCK
	close(sock);
#endif
	free(buffer);
	return NULL;
}

