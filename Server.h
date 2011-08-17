#ifndef _SERVER_H
#define _SERVER_H
typedef struct {
	char *ext;
	char *filetype;
} extension;

void *server(void *socket);
void server_setUtility(bool ut);
#endif
