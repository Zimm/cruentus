#include <iostream>
#include "Socket.h"

using namespace std;

int main(int argc, char **argv) {

	Socket *sock = new Socket();

	sock->bind(7777);
	sock->listen();
	sock->accept();
	
	delete sock;

	return 0;

}

