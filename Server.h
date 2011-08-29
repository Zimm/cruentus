#include <string>
#include <vector>
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

typedef struct {
	std::string domain;
	std::string path;
} serverPath;

void server_readConf(const char *file);
void *server(void *socket);
void server_setUtility(bool ut);
void server_setCrux(bool crux);
void server_setLogging(bool log);
#endif
