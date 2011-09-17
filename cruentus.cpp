#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "Socket.h"
#include "Server.h"

using namespace std;

static bool httpServer = false;

void pipesignal(int sig) {
        cout << "Tried to call to a closed pipe" << endl;
}

int main(int argc, char **argv) {
	uint16_t aport = 7331;
	string *conf_file = new string("/etc/cruentus/cruentus.conf");

	for (int i = 1; i < argc; i++) {
	
		if (strncmp(argv[i], "--root=", 7) == 0) {
			string root(argv[i]);	
			root = root.substr(7);
			if (strncmp(root.c_str(), "~", 1) == 0) {
				root = root.substr(1);
				char *home = getenv("HOME");
				root = string(home).append(root);
			}
			cout << "Changing root to " << root << endl;
			if (chdir(root.c_str()) != 0) {
				cout << "Failed to change to root " << root << endl;
			}
		} else if (strncmp(argv[i], "--port=", 7) == 0) {
			string port(argv[i]);
			port = port.substr(7);
			cout << "Changing port to " << port << endl;
			aport = atoi(port.c_str());
		} else if (strncmp(argv[i], "--utility", 9) == 0) {
			cout << "Setting utility on" << endl;
			server_setUtility(true);
		} else if (strncmp(argv[i], "--httpserver", 12) == 0) {
			cout << "Creating http web server...." << endl;
			httpServer = true;
		} else if (strncmp(argv[i], "--crux", 6) == 0) {
			cout << "Creating crux server...." << endl;
			server_setCrux(true);
		} else if (strncmp(argv[i], "--daemon", 8) == 0) {
			int forker = fork();
			if (forker != 0) {
				cout << "Forking into "<< forker << " process" << endl;
				exit(0);
			}
		} else if (strncmp(argv[i], "--conf=", 7) == 0) {
			delete conf_file;
			string conf(argv[i]);
			conf = conf.substr(7);
			if (strncmp(conf.c_str(), "~", 1) == 0) {
                                conf = conf.substr(1);
                                char *home = getenv("HOME");
                                conf = string(home).append(conf);
                        }
			conf_file = new string(string(argv[i]).substr(7));
			cout << "Changed conf file to " << conf_file;
		} else if (strncmp(argv[i], "--log", 5) == 0) {
			server_setLogging(true);
			cout << "Server is now being logged" << endl;
		} else if (strncmp(argv[i], "--echo", 6) == 0) {
			server_setEcho(true);
			cout << "Created echo server" << endl;
		} else {
			cout << "Illiegal option " << argv[i] << endl;
			exit(-1);
		}
		
	}
	signal(SIGPIPE, pipesignal);
	if (httpServer) {
		server_readConf(conf_file->c_str());
	}
	
	delete conf_file;

	Socket *sock = new Socket();

	sock->bind(aport);
	sock->listen(BACKLOG);
	if (httpServer)
		sock->accept(server);
	else
		sock->accept();
	
	delete sock;

	return 0;

}

