#ifndef CHANNEL_HEADER_
#define CHANNEL_HEADER_

#include <pthread.h>

struct channel_t {
	int* buf;
	int len;
	int read;
	int write;
	pthread_mutex_t guard;
	pthread_cond_t w_cond;
	pthread_cond_t r_cond;
};

channel_t* newChannel(int n);
void send(channel_t*, int);
int recv(channel_t*);

#endif

