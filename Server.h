#ifndef _SERVER_H
#define _SERVER_H
typedef struct {
	char *ext;
	char *filetype;
} extension;
typedef struct {
	char *ext;
	int type;
} cruxExt;

void *server(void *socket);
void server_setUtility(bool ut);
void server_setCrux(bool crux);
#endif
