#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h>

#define DEFAULT_PORT 5001
#define MAX_REQUESTS 5

typedef int socket_t;

/**
 * Prints a text containing informations on how to use this program
 */
void printCorrectUsage(char const * const progname);

/**
 * Writes the needed metadata for sending to a server into a given struct.
 * The input 'info' is either a nullterminated string containing <IP-Address> 
 * or <IP-Address>:<Port>
 */
int determineServAddr(char const * const info, struct sockaddr_in *dest);

/**
 * Sends the needed data to the specified server and writes the received string to the given pointer.
 */
int transferData(char const * const input, struct sockaddr_in const * const server, char **receivedStr);

/**
 * Only used to send data to the given host. Returns the socketId where the received data can be read from.
 * dataSize describes the requested byte amount.
 */
socket_t sendData(char const * const input, u_int16_t dataSize, struct sockaddr_in const * const server);

/**
 * Extracts data from a socket and writes a newly allocated string to dest.
 * Returns the size of the written dest.
 * esteimatedDataSize descibes the datasize that was requested.
 */
long int receiveData(socket_t received, char **dest, u_int8_t estimatedDataSize);

/**
 * Basically splits a string at the last occurance of ':' looking like this:
 * <dest1>:<dest2>
 */
void extractFromFormat(char const * const src, char * dest1, char *dest2);

/**
 * Finds the last position of ':' in a given nullterminated string. Returns 0 if not found.
 */
size_t findDelimiter(char const * const src);



#endif 