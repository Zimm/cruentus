#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>

#define FATAL() __builtin_trap()

struct sock_conn {
    int fd;
    struct sockaddr_in *saddr;
};

static int main_sock = -1;
static int timeout = -1;

void *socket_connection(struct sock_conn *a) {
    char buf[1024];
    int ret;
    struct timeval tv;
    struct linger lingading;
    lingading.l_onoff = 1;
    lingading.l_linger = 5;
    if (setsockopt(a->fd, SOL_SOCKET, SO_LINGER_SEC, (const void*)&lingading, (socklen_t)sizeof(struct linger)) != 0) {
        perror("setsockopt1");
        return NULL;
    }
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    if (setsockopt(a->fd, SOL_SOCKET, SO_SNDTIMEO, (const void*)&tv, (socklen_t)sizeof(struct timeval)) != 0) {
        perror("setsockopt2");
        return NULL;
    }
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    if (setsockopt(a->fd, SOL_SOCKET, SO_RCVTIMEO ,(const void*)&tv, (socklen_t)sizeof(struct timeval)) != 0) {
        perror("setsockopt3");
        return NULL;
    }
    printf("Opened socket: %d\n", a->fd);
    write(a->fd, "hi\n",3);
    while (1) {
        if ((ret = read(a->fd, buf, 1023)) < 0) {
            if (errno == EAGAIN) {
                fprintf(stderr, "Connection to %s:%d timed out\n", inet_ntoa(a->saddr->sin_addr), ntohs(a->saddr->sin_port));
            } else {
                perror("read");
            }
            break;
        }
        buf[ret] = '\0';
        if (ret == 0) {
            break;
        }
        write(a->fd, buf, ret);
    }
    printf("Closing socket: %d\n", a->fd);
    if (shutdown(a->fd, SHUT_RDWR) != 0) {
        if (errno != ENOTCONN)
            perror("shutdown");
    }
    if (close(a->fd) != 0) {
        perror("close");
    }
    free(a->saddr);
    free(a);
    return NULL;
}

void exit_handler(void) {
    if (main_sock != -1) {
        shutdown(main_sock, SHUT_RDWR);
        close(main_sock);
    }
}

void usage(char *me) {
    fprintf(stderr,
            "Usage: %s [-p port] [-a address] [-t timeout] [-h]\n"
            "\t-p port: Specify the port which to bind the server (default: 3000)\n"
            "\t-a address: Specify the address which to bind the server on (default: INADDR_ANY)\n"
            "\t-t timeout: Specify the timeout length when someone connects (default: 30) NOTE: in seconds\n"
            "\t-h: Halp!\n", me);
    exit(1);
}

int main(int argc, char *argv[]) {
    atexit(exit_handler);
    int insock, ch;
    struct sockaddr_in saddr, *inaddr;
    struct sock_conn *scon;
    pthread_t athread;
    socklen_t inaddrlen;
    uint16_t port = 0;
    char *bindaddr = NULL;
    while ((ch = getopt(argc, argv, "a:p:t:h")) != -1) {
        switch (ch) {
            case 'a':
                bindaddr = (char *)malloc(strlen(optarg) + 1);
                strcpy(bindaddr, optarg);
                break;
            case 'p':
                port = (uint16_t)atoi(optarg);
                break;
            case 't':
                timeout = atoi(optarg);
                break;
            case 'h':
                fprintf(stderr, "Help is on the way!\n");
            case '?':
            default:
                usage(argv[0]);
                break;
        }
    }
    if (timeout == -1)
        timeout = 30;
    if ((main_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        FATAL();
    }
    bzero(&saddr, sizeof(struct sockaddr_in));
    saddr.sin_len = sizeof(struct sockaddr_in);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port != 0 ? port : 3000);
    saddr.sin_addr.s_addr = bindaddr ? inet_addr(bindaddr) : INADDR_ANY;
    if (bind(main_sock, (const struct sockaddr *)&saddr, saddr.sin_len) != 0) {
        perror("bind");
        FATAL();
    }
    fprintf(stdout, "Starting server on %s:%d\n", bindaddr ? : "*", port != 0 ? port : 3000);
    if (listen(main_sock, 10) != 0) {
        perror("listen");
        FATAL();
    }
    while (1) {
        inaddrlen = sizeof(struct sockaddr_in);
        inaddr = (struct sockaddr_in*)malloc(inaddrlen);
        if ((insock = accept(main_sock, (struct sockaddr*)inaddr, &inaddrlen)) < 0) {
            perror("accept");
            FATAL();
        }
        scon = (struct sock_conn*)malloc(sizeof(struct sock_conn));
        scon->fd = insock;
        scon->saddr = inaddr;
        if (pthread_create(&athread, NULL, (void *(*)(void*))socket_connection, scon) != 0) {
            perror("pthread_create");
            FATAL();
        }
    }
    return 0;
}

