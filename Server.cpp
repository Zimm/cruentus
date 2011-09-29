#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <fstream>
#include <time.h>
#include <algorithm>
#include <vector>
#include <errno.h>
#include <cerrno>
#include "Socket.h"
#include "Server.h"

using namespace std;

static bool utility_ = false;
static bool crux_ = false;
static bool logging_ = false;

extension extensions[] = {
        {(char *)".j", (char *)"text/javascript"},
        {(char*)".gif", (char*)"image/gif" },
        {(char*)".jpg", (char*)"image/jpeg"},
        {(char*)".jpeg",(char*)"image/jpeg"},
        {(char*)".png", (char*)"image/png" },
        {(char*)".zip", (char*)"image/zip" },
        {(char*)".gz",  (char*)"image/gz"  },
        {(char*)".tar", (char*)"image/tar" },
        {(char*)".htm", (char*)"text/html" },
        {(char*)".html",(char*)"text/html" },
	{(char *)".hcrux",(char *)"text/html" },
        {(char *)".css", (char *)"text/css"},
        {(char *)".js", (char *)"text/javascript"},
        {(char *)".mp4", (char *)"video/mp4"},
        {(char *)".tiff", (char *)"image/tiff"},
        {(char *)".pdf", (char *)"application/pdf"},
        {0,0} };



cruxExt cruxExtenstions[]= {
	{(char *)".hcrux", kCruxText},
	{(char *)".crux", kCruxSock},
	{(char *)".php", kCruxSock},
	{0,kCruxUnknown}};


static vector<serverPath> *servers = new vector<serverPath>();

void server_setUtility(bool ut) {
	utility_ = ut;
}
void server_setCrux(bool crux) {
	crux_ = crux;
}

void server_setLogging(bool log) {
	logging_ = log;
}

void server_readConf(const char *path) {
	
	ifstream in(path);
	char *data = new char[1024];
	
	while (in) {
		in.getline(data, 1024);
		if (!in || data[0] == 0)
			continue;
		string dd(data);
		if (dd[0] == '#')
			continue;
		size_t first = dd.find("<*server*>");
		size_t second = dd.find("<*server*>", first+11/*lengthof <*server*> ++*/);
		string stuff = dd.substr(first+10, (second - (first+10)));
		size_t sppl = stuff.find(" ");
		string domain = stuff.substr(0, sppl);
		string path = stuff.substr(sppl+1);
		serverPath server;
		server.domain = domain;	
		server.path = path;
		servers->push_back(server);
#ifdef DEBUG
		cout << "Found server " << server.domain << " with path " << server.path << endl;
#endif	
	}
	delete data;
	in.close();
}


void replace(string &str, const string &find_what, const string &replace_with)
{
	string::size_type pos=0;
	while((pos=str.find(find_what, pos))!=string::npos)
	{
		str.erase(pos, find_what.length());
		str.insert(pos, replace_with);
		pos+=replace_with.length();
	}
}

void *socket_forward(void *sockets);

static vector<int> *readyToClose = new vector<int>;


void *server(void *socket) {
	int sock = *((int*)socket);
	Socket *asock_= new Socket(sock);
#ifdef DEBUGSOCK
	cout << pthread_self() << ": Got socket " << sock << endl;
#endif
	int bufsize=1024;
	string buffer("");
	int leggs = 0;
	do {
		char *abuff = (char *)calloc(1,bufsize);
		leggs = recv(sock,abuff,bufsize,0);
		if (strlen(abuff) == 0)
			continue;
		buffer += abuff;
		free(abuff);
	} while (leggs > 0 && buffer.find("\r\n\r\n") == string::npos);

	if (logging_) {
		time_t rtime;
		time(&rtime);
		string ts = string(ctime(&rtime));
		cout << "[" << ts.substr(0,ts.length()-1) << "] Request:" << endl << buffer << endl << endl;
	}
#ifdef DEBUG
	cout << "Request:" << endl << buffer << endl<<"Done"<<endl;
#endif
	//get the url they want.....
	if (strncmp(buffer.c_str(), "GET ", 4) && strncmp(buffer.c_str(),"get ",4) && strncmp(buffer.c_str(), "POST ", 5) && strncmp(buffer.c_str(), "post ", 5)) {
		asock_->send(string("Unknown protocol D:"));
		close(sock);
		delete asock_;
		return NULL;
	}		
	
	string request(buffer);
	
	request = request.substr(4);
        if (request[0] == ' ')
                request = request.substr(1);
        size_t pos = request.find(" HTTP/1");
        //NOTE MUST ADD IF NPOS
        request = request.substr(0,pos);

	if (servers->size() > 0) {
		string aserver(buffer);
		size_t hostp = aserver.find("Host:");
		size_t newlp = aserver.find("\r\n", hostp);
		if (hostp == string::npos || newlp == string::npos) 
			goto skipWho;
		aserver = aserver.substr(hostp, newlp-hostp);
		aserver = aserver.substr(5);
		if (aserver[0] == ' ')
			aserver = aserver.substr(1);
		
		aserver += request;
		if (aserver.find("aboutcrux") == 0) {
			asock_->send(string("HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n<html><body>Cruentus:<br/>The bloodiest web server out there<br/>Version 0.1a</body></html>"));
			close(sock);
			delete asock_;
			return NULL;
		}	
		size_t matches = 0;
		string path("./");
		string removePath("");
		for (unsigned int i = 0; i < servers->size(); ++i) {
			string currentDomain((*servers)[i].domain);
			size_t matchest = aserver.find(currentDomain);
			if (matchest == string::npos && currentDomain.compare("*") != 0)
				continue;
		//	size_t matchl = currentDomain.length();
		
			string apath("");
			size_t matchl = 0;
			bool startedPath =false;
			for (unsigned int ii =0; ii < currentDomain.length();++ii) {
				if (currentDomain[ii] == aserver[matchest+ii]) {
					matchl++;
					if (startedPath) {
						apath += currentDomain[ii];
					} else if (currentDomain[ii] == '/') {
						startedPath = true;
						apath += currentDomain[ii];
					}
				}
			}
		
			if (matchl > matches) {
				path = (*servers)[i].path;
				matches = matchl;
				removePath = apath;
			}
		}
		size_t wherecolon = path.find(":");
		if (wherecolon != string::npos && wherecolon != path.length()-1) {
			string ports = path.substr(wherecolon+1);
			uint16_t out;
			stringstream as;
			as << ports;
			as >> out;
			string place1 = wherecolon == 0 ? "localhost" : path.erase(wherecolon);
			cout << "Forwarding to place" << place1 << ":" << out << endl;
			// then it wants to port forward, hold on boys
			Socket *unSock = new Socket();			
			unSock->connect((char *)place1.c_str(), out);
			unSock->send(buffer);
			int *stuff = (int *)calloc(1,sizeof(int)*2);
			stuff[0] = sock;
			stuff[1] = *(unSock->socket_);
			int *stuff1 = (int *)calloc(1,sizeof(int)*2);
			stuff1[0] = *(unSock->socket_);
			stuff1[1] = sock;
			pthread_t newThread;
			pthread_t newThread1;
			if (pthread_create(&newThread, NULL, socket_forward, stuff)) {
				fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", errno);
				perror("THREAD ERROR");
				::close(*(unSock->socket_));
				::close(sock);
				delete unSock;
				delete asock_;
	        	return NULL;
			}
			if (pthread_create(&newThread1, NULL, socket_forward, stuff1)) {
				fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", errno);
				perror("THREAD ERROR");
				::close(*(unSock->socket_));
				::close(sock);
				delete unSock;
				delete asock_;
	        	return NULL;
			}
			delete unSock;
			delete asock_;
        	return NULL;
		}
		if (chdir(path.c_str()) != 0) {
			cout << "Failed to chdir(" << path << ")" << endl;
		} 
		
		if (removePath.length() > 1) {
			size_t where = request.find(removePath);
			if (where != string::npos) {
				request.erase(where, removePath[removePath.length()-1]=='/'? removePath.length()-1:removePath.length());
			}
		}
		
	}
skipWho:
	
#ifdef DEBUGFILE
	cout << "Requesting " << request << endl;
#endif
	if (!utility_ && strcmp(request.c_str(), "/") == 0)
		request = !crux_ ? "/index.html":"/index.hcrux";
	else if (!utility_ && strncmp(request.c_str(), "/?", 2) == 0)
		request.insert(1, string(!crux_?"index.html":"index.hcrux"));
#ifdef DEBUG
	cout << "Trying to get " << request << endl;
#endif 
	struct stat st;
	string requestp = "";
	if (crux_) {
		size_t posp = request.find("?");
		if (posp == string::npos)
			goto skipcrux;
		string requestparam = request.substr(posp+1);
		size_t space = 0;
		while (space != string::npos) {
			space = requestparam.find("%20");
			if (space != string::npos)
				requestparam.replace(space, 3, " ");
		}
		requestp = requestparam;
		request.erase(posp);
	}
skipcrux:
	request.insert(0,string("."));

	stat(request.c_str(), &st);


	if (!utility_ && S_ISDIR(st.st_mode) && !(crux_ && request.find(".crux")!=string::npos) && !(crux_ && request.find(".php")!=string::npos)) {
		if (request[request.length()-1] == '/')
			request += !crux_?"index.html":"index.hcrux";
		else
			request += !crux_?"/index.html":"/index.hcrux";
	}

	if (stat(request.c_str(), &st) != 0) {
		if (request.length() >= 11 && request.substr(request.length()-11).compare(string("index.hcrux")) == 0) {
			request = request.substr(0,request.length()-4).append(string("tml"));
		}
	}


	
	if ((stat(request.c_str(), &st) != 0 || S_ISDIR(st.st_mode)) && !(crux_ && request.find(".crux")!=string::npos) && !(crux_ && request.find(".php")!=string::npos)) {
//skipIndexCheck:
#ifdef DEBUGFAIL
		cout << "failed to get resource " << request << endl;
#endif
		if (!utility_) {
			asock_->send(string("HTTP/1.1 404 Not Found\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n<html><body>NotFound<br/>Goodbye World<br/></body></html>"));
		} else {
		
			DIR *dp;
			struct dirent *ep;     
			dp = opendir (request.c_str());
			if (dp != NULL) {
				asock_->send(string("HTTP/1.1 200 OK\r\n"));
				asock_->send(string("Content-Type: text/html; charset=UTF-8\r\n\r\n"));
				asock_->send(string("<html><body>"));
				while ((ep = readdir (dp)) != NULL) {
					asock_->send(string("<a href=./"));
					asock_->send(string(ep->d_name));
					if (ep->d_type == DT_DIR) {
						asock_->send(string("/"));
					}
					asock_->send(string(">"));
					asock_->send(string(ep->d_name));
					asock_->send(string("</a><br/>"));
				}	
				asock_->send(string("</body></html>"));
				closedir (dp);
			} else {
#ifdef DEBUGFAIL
				cout << "Failed to get resource dir" << endl;
#endif
				asock_->send(string("HTTP/1.1 404 Not Found\r\n"));
				asock_->send(string("Content-Type: text/html; charset=UTF-8\r\n\r\n"));
				asock_->send(string("<html><body>Goodbye World</body></html>"));
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
		
		header += "Server: cruentus/0.1a\r\n";

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
		size_t currentCrux = -1;
		if (crux_) {
			crux_t cruxType = kCruxUnknown;
			for (int i=0;cruxExtenstions[i].ext != 0;i++) {
				size_t len = strlen(cruxExtenstions[i].ext);
				if (!strncmp(request.substr(leng-len).c_str(), cruxExtenstions[i].ext, len)) {
                                	cruxType =cruxExtenstions[i].type;
                                	break;
                        	} else if (request.find(".php?") != string::npos) {
					cruxType = kCruxSock;
					break;
				}
			}
			switch (cruxType) {
				case kCruxSock:{
					Socket *unSock = new Socket(AF_UNIX, SOCK_STREAM, 0);
					string tmpr(request);
					if (tmpr[tmpr.length()-1]=='/')
						tmpr+="sock";
					else
						tmpr+="/sock";
					unSock->connect((char *)tmpr.c_str());
					unSock->send(buffer);
					
					int bs = 1024;
					int rt = 0;
					do {
						char *aabs = (char *)calloc(1,bs+1);
						rt = recv(*(unSock->socket_),aabs,bs,0);
						aabs[bs] = '\0';
						if (strlen(aabs) == 0)
							continue;
						asock_->send(string(aabs));
						free(aabs);
					} while (rt > 0);
					
					delete unSock;
					break;
				}
				case kCruxText:{
					header+= "\r\n\r\n";
					asock_->send(header);
					ifstream fstream(request.c_str(),ifstream::in);
					size_t place = 0;
					char ptest[255];
					while (!fstream.eof()) {
						fstream.getline(ptest, 255);
						if (!fstream || ptest[0]==0)
							continue;
						string sptest(ptest);
						size_t aplace = 0;
						while (aplace != string::npos) {
							place = aplace = sptest.find("<*", aplace);
							if (place != string::npos) {
								size_t place2 = sptest.find("*>",aplace);
								if (place2 != string::npos) {
									int dp = (place2) - (place + 2);
									size_t crpl = requestp.find("?", ++currentCrux);
									string arequestp("");
									if (crpl != string::npos) {
										arequestp = requestp.substr(currentCrux, crpl-currentCrux);
										currentCrux = crpl;
									} else if (requestp.find("?", currentCrux+1) == string::npos) {
										arequestp = requestp.substr(currentCrux);
										currentCrux = requestp.length()-1;
									}
									if (dp > 0) {
										string def = sptest.substr(place+2,dp);
#ifdef DEBUG
										cout << "Default is " <<def << endl;
#endif	
										sptest.erase(place, (place2+2)-place);
										sptest.insert(place, (arequestp.length()>0 ? arequestp : def));
									} else {
										sptest.erase(place, 3);
										sptest.insert(place, arequestp);
									}
								}
							}
						}
						asock_->send(sptest);
					}
					fstream.close();
					break;
				}
				default:
					goto branchRegular;
					break;
			};
		} else {
branchRegular:
			header += "Content-Length: ";
		
			ostringstream result;
			result << st.st_size;
			header += result.str();
			
			header += "\r\n\r\n";
#ifdef DEBUGFILE
			cout << "Sending " << header << " for " << request << endl;
#endif
		
			asock_->send(header);
			asock_->sendFile(request);

		}
	
	}
#ifdef DEBUGSOCK
	cout << pthread_self() << ": Closing socket" << sock << endl;
#endif
#ifndef TESTSOCK
	close(sock);
#endif
	delete asock_;
	return NULL;
}

void *socket_forward(void *sockets) {
	
	int *fds = (int *)sockets;
	
	int from = fds[0];
	int to = fds[1];
	
	cout << "Started forwarding... " << from << " and " << to << endl;
		
	char buf[8192];
	ssize_t rc;
	ssize_t amt;
	ssize_t offset;

	while (1) {
		rc = read(from, buf, sizeof(buf));
		if (rc <= 0) {
			if (rc == 0) break;
			if (errno == EINTR || errno == EAGAIN) continue;
			perror("read error");
			break;
		}
		offset = 0;
		amt = rc;
		while (amt) {
			rc = write(to, buf+offset, amt);
			if (rc < 0) {
				if (errno == EINTR || errno == EAGAIN) continue;
				perror("write error");
				break;
			}
			offset += rc;
			amt -= rc;
		}
	}
	
	
	cout << "Finishing forwarding " << from << " and " << to << endl;
		
	vector<int>::iterator result1, result2;

	result1 = std::find(readyToClose->begin(), readyToClose->end(), fds[0]);
	result2 = std::find(readyToClose->begin(), readyToClose->end(), fds[1]);

	if (result1 != readyToClose->end()) {
		close(fds[0]);
	} else {
		readyToClose->push_back(fds[0]);
	}

   	if (result2 != readyToClose->end()) {
		close(fds[1]);
	} else {
		readyToClose->push_back(fds[1]);
	}
	//close(fds[0]);
	//close(fds[1]);
	free(fds);
	return NULL;
}


