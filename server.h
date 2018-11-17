#ifndef SERVER_H
#define SERVER_H

#include <sys/types.h>
#include <signal.h>
#include "worker.h"

#define DEFAULT_PORT 5001
#define DEFAULT_QUEUE_SIZE 5
#define TRUE 1

/**
 * Signalhandler handling incoming SIG_INT signals.
 * As soon as a SIG_INT signal rises the server will shut 
 * down properly closing all eventually remaining connections.
 */
void sigIntHandle(int signum, siginfo_t *info, void *ptr);

/**
 * Signalhandler handling incoming SIGCHLD signals. 
 * As soon as a Child exits, the remaining zombie process 
 * will be handled and removed correctly
 */
void sigChldHandle(int signum, siginfo_t *info, void *ptr);

/**
 * Installs any used custom signalhandlers into the current process.
 */
void installSigHandlers();

/**
 * Creates, binds and sets a socket to passive using the given 
 * portnumber and maximum queuesize.
 */
socket_t createSocket(u_int16_t portno, size_t queueSize);

#endif