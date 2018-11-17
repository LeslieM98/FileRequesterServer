#ifndef WORKER_H
#define WORKER_H

typedef int socket_t;

/**
 * Starts a worker that fetches request data from a socket and returns according data.
 */
void startWorker(socket_t sockedId);

#endif
