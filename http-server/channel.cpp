#include "channel.h"
#include <cstdlib>
#include <pthread.h>

channel_t* newChannel(int n) {
	channel_t* chan = (channel_t*) malloc(sizeof(channel_t));
	chan->buf = (int*) malloc(n * sizeof(int));
	chan->len = n;
	chan->read = 0;
	chan->write = 0;
	pthread_mutex_init(&chan->guard, NULL);

	return chan;
}

void freeChannel(channel_t* chan) {
	free(chan->buf);
	pthread_mutex_destroy(&chan->guard);
	free(chan);
}

void send(channel_t* chan, int value) {
	pthread_mutex_lock(&chan->guard);

	int advanced;
	while (true) {
		advanced = (chan->write + 1) % chan->len;
		if (advanced == chan->read) {
			// The channel is full
			pthread_cond_wait(&chan->w_cond, &chan->guard);
		} else {
			break;
		}
	}

	chan->buf[chan->write] = value;
	chan->write = advanced;

	pthread_cond_signal(&chan->r_cond);
	pthread_mutex_unlock(&chan->guard);
}

int recv(channel_t* chan) {
	pthread_mutex_lock(&chan->guard);

	while (chan->read == chan->write) {
		// The channel is empty
		pthread_cond_wait(&chan->r_cond, &chan->guard);
	}

	// TODO: there is something called a conditional wait. We should consider this here
	// See https://github.com/tylertreat/chan/blob/master/src/chan.c
	// pthread_cond_wait
	// We can signal to a waiting reader when we wrote to the channel using pthread_cond_signal
	// And vise verca
	// If somebody is waiting, then we can send a signal using the conditional variable

	int retval = chan->buf[chan->read];
	chan->read = (chan->read + 1) % chan->len;

	pthread_cond_signal(&chan->w_cond);
	pthread_mutex_unlock(&chan->guard);

	return retval;
}
