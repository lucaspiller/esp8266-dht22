#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_

extern int tcp_client_send(char* target, int port, char* data, void(*callback)(int));

#define TCP_CLIENT_SENDING 0
#define TCP_CLIENT_SENT    1
#define TCP_CLIENT_ERROR   2

#endif
