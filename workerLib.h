#ifndef WORKER_LIB_H
#define WORKER_LIB_H

#include "worker.h"

#include <sys/types.h>


#define REQ_BYTES 2
#define FILE_BYTES 2
#define IN_OVERHEAD (REQ_BYTES + FILE_BYTES) 

#define STAT_BYTES 1
#define READ_BYTES 2
#define OUT_OVERHEAD (STAT_BYTES + READ_BYTES)


static pid_t currentPid;


/**
 * Allocates a memory field with fixed size, sets the whole field to 0.
 */
u_int8_t *initBuffer(size_t size);

/**
 * Allocates a new nullterminated string containing the IP address 
 * of the client connected to the given socket. Returns the start of said string.
 */
char *getClientIP(socket_t socketfd);


/**
 * Reads the requested n-bytes from an active socket and returns said.
 */
u_int16_t getRequestedNBytes(socket_t requestSocket);

/**
 * Reads the length of the path of the requested file from an active socket.
 * Returns said length
 */
u_int16_t getFilePathLength(socket_t requestSocket);

/**
 * Reads the actual file path from an active file path and returns a newly 
 * allocated nullterminated string containing said string.
 */
char *getFilePath(socket_t, u_int16_t pathLength);

/**
 * Assembles a package containing all needed informations and writes it to dest.
 */
void preparePackage(u_int8_t status, u_int16_t contentSize, u_int8_t *content, u_int8_t *dest);

/**
 * Returns a package back to the requesting client containing all needed informations.
 */
void returnPackage(u_int8_t *package, size_t packageSize, socket_t socket);

/**
 * Reads the requested size of bytes from a file and writes it to dest. 
 * Returns the actual read bytes.
 */
size_t getFileContent(char *filename, void *dest, size_t size);


#endif
