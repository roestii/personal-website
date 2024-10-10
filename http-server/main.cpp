#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "channel.h"
#include "vector.h"

// TODO: How do we support multiple connections per thread? Switch on the read per connection?
// But how do we know whether data is available? We probably have to use select to check whether 
// data is available and read for a given connection. When there is no data available and we already 
// read data, then we should parse out the http content.
//
void* handleConnections(void* args) {
	channel_t* chan = (channel_t*) args;
	// We would need a readBuf for each connection. We might be able to just parse the content we 
	// currently have and assemble the result after reading from the socket again... i don't know right now
	vector_t<uint8_t>* readBuf = newVector<uint8_t>(65536);

	while (true) {
		int sfd = recv(chan);
		// Maybe we should use the buf inside the vec directly
		uint8_t buf[1024];
		fd_set set;

		// TODO: Refactor this to support multiple connections per thread using 
		// select.
		while (true) {
			int n = read(sfd, &buf, sizeof(buf));

			if (n == -1 || n == 0) {
				if (close(sfd) == -1) {
					fprintf(stderr, "Failed to close socket\n");
				}

				fprintf(stdout, "connection closed\n");
				break;
			}

			pushMany(readBuf, buf, n);

			while (true) {
				FD_SET(sfd, &set);
				struct timeval t = {0};
				int retval = select(sfd + 1, &set, NULL, NULL, &t);

				if (retval == -1) {
					fprintf(stderr, "Failed to select\n");
					break;
				} else if (retval) {
					// Here we can introduce multiple connections
					if (FD_ISSET(sfd, &set)) {
						printf("is set\n");
					}

					int n = read(sfd, &buf, sizeof(buf));
					if (n == -1 || n == 0) {
						if (close(sfd) == -1) {
							fprintf(stderr, "Failed to close socket\n");
						}

						fprintf(stdout, "connection closed\n");
						break;
					}

					pushMany(readBuf, buf, n);
				} else {
					break;
				}
			}

			printf("%.*s", readBuf->len, readBuf->buf);
		}
	}

	return NULL;
}

int main(void) {
	// TODO: how do we kill the server gracefully?
	const int N_CONNS = 1024;
	const int N_THREADS = 8;
	channel_t* chan = newChannel(1024);
	pthread_t ths[N_THREADS];

	for (int i = 0; i < N_THREADS; ++i) {
		if (pthread_create(ths + i, NULL, handleConnections, (void*) chan) == -1) {
			fprintf(stderr, "Failed to spawn thread\n");
			return -1;
		}
	}


	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	int optval = 1;
	setsockopt(sfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
	struct sockaddr_in addr = {0};
	struct sockaddr_in peerAddr = {0};

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(8080);

	if (bind(sfd, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
		fprintf(stderr, "Failed to bind\n");
		return -1;
	};

	if (listen(sfd, N_CONNS) == -1) {
		fprintf(stderr, "Failed to listen\n");
		return -1;
	}

	while (true) {
		socklen_t peerAddrSize = sizeof(peerAddr);
		int cfd = accept(sfd, (struct sockaddr*) &peerAddr, &peerAddrSize);

		if (cfd == -1) {
			return -1;
		}

		send(chan, cfd);
	}

	if (close(sfd) == -1) {
		fprintf(stderr, "Failed to close socket\n");
		return -1;
	}

	return 0;
}

