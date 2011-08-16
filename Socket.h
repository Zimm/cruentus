#include <stdlib.h>
#include <stdio.h>


class Socket {
                int *socket_;
        public:
                Socket(int domain, int type, int protocol);
		Socket();
                ~Socket();
                void bind();
                void bind(uint16_t port);
                void listen();
                void listen(int backlog);
                void accept(void *(*start_routine)(void*));
                void accept();
};
