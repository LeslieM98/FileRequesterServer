
#include "client.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>



int main(int argc, char const *argv[]){
    if(argc < 3 || argc > MAX_REQUESTS + 2){
        fprintf(stderr, "Incorrect argumentcount.\n");
        printCorrectUsage(argv[0]);
    }

    struct sockaddr_in addrInfo;
    if(determineServAddr(argv[1], &addrInfo)){
        return 1;
    }

    // Main receive loop
    char *received = NULL;
    int receivedSize;
    for(int i = 2; i < argc; i++){
        printf("Trying to recieve: %s\n", argv[i]);
        receivedSize = transferData(argv[i], &addrInfo, &received);
        if (receivedSize < 0){
            continue;
        }
        printf("Received %d byte:\n", receivedSize);
        for(size_t j = 0; j < receivedSize; j++){
            putc(received[j], stdout);
        }
        putc('\n', stdout);
        free(received);
    }

    return 0;
}

int determineServAddr(char const * const info, struct sockaddr_in *dest){
    const char containsPort = findDelimiter(info);

    char ipaddr[17];
    char buff[10];
    int port = -1;

    if(containsPort){
        extractFromFormat(info, ipaddr, buff);
        port = atoi(buff);
    } else {
        strcpy(ipaddr, info);
        port = DEFAULT_PORT;
    }

    printf("Connecting to Server (IP: %s, Port: %d)\n", ipaddr, port);

    if(port == -1){
        fprintf(stderr, "Error while reading Serverinformations.\n");
        return 1;
    }

    dest->sin_family = AF_INET;
    dest->sin_port = htons(port);
    inet_aton(ipaddr, &dest->sin_addr);
    return 0;
}

int transferData(char const * const input, struct sockaddr_in const * const server, char** receivedStr){

    // Extract requested file and requested bytes
    size_t filenameSize = findDelimiter(input);
    char filename[filenameSize+1];
    char buff[8];
    memset(buff, 0, 8);
    extractFromFormat(input, filename, buff);
    u_int16_t requestedSize = atol(buff);



    socket_t serverConnection = sendData(filename, requestedSize, server);
    if(serverConnection < 0){
        return -1;
    }
    int writtenSize = receiveData(serverConnection, receivedStr, requestedSize);
    if(writtenSize < 0){
        return -2;
    }
    return writtenSize;
}

socket_t sendData(char const * const input, const u_int16_t dataSize, struct sockaddr_in const * const server){
    const u_int16_t fileNameLen = strlen(input);
    if(fileNameLen == 0){
        fprintf(stderr, "No filename or filesize given\n");
        return 0;
    }

    socket_t serverConnection = socket(AF_INET, SOCK_STREAM, 0);
    if(serverConnection < -1){
        perror("Could not create Socket");
        return -1;
    }

    u_int8_t packet[4 + fileNameLen];
    ((u_int16_t*) packet)[0] = dataSize;
    ((u_int16_t*) packet)[1] = fileNameLen;

    strncpy(&packet[4], input, fileNameLen);

    if(connect(serverConnection, (struct sockaddr*) server, sizeof(struct sockaddr_in)) < 0){
        perror("Could not connect to server");
        return -2;
    }

    send(serverConnection, packet, sizeof(packet), 0);
    return serverConnection;
}

long int receiveData(socket_t received, char **dest, const u_int8_t estimatedDataSize){
    const size_t packetSize = 3 + received;
    u_int8_t receivedPacket[3+estimatedDataSize];

    // receive package
    const ssize_t actualBytesReturned = recv(received, receivedPacket, 20, 0);
    close(received);
    if(actualBytesReturned == 0){
        perror("No package returned.");
        return -5;
    } else if(actualBytesReturned < 0){
        perror("Error while receiving data.");
        return -6;
    }

    // Check status of returned package
    switch(receivedPacket[0]){
        case 1:
            fprintf(stderr, "No permissions or file not found.\n");
            return -1;
            break;
        default:
            break;
    }

    // Extract size of returned string
    u_int16_t *size = (u_int16_t *) &receivedPacket[1];
    if(*size == 0){
        fprintf(stderr, "0 bytes from file returned.\n");
        return -2;
    }

    // Write back received string
    *dest = malloc(*size+1);
    strncpy(*dest, &receivedPacket[3], *size);
    (*dest)[*size] = 0;

    return *size;
}

void extractFromFormat(char const * const src, char * dest1, char * dest2){
    const char delim = findDelimiter(src);
    const char nullterm = strlen(src);

    const char tmp = nullterm - delim;
    strncpy(dest2, &(src[delim + 1]), tmp);
    dest2[tmp] = 0;
    strncpy(dest1, src, delim);
    dest1[delim] = 0;
}

size_t findDelimiter(char const * const src){
    size_t ret = 0;

    int i = strlen(src);
    do{
        if(src[i] == ':'){
            ret = i;
            break;
        }
        i--;
    } while (i != 0);
    return ret;
}

void printCorrectUsage(char const * const progname){
    printf("Correct usage:\n");

    printf("$%s <IP-Address>:<Port>", progname);
    for(int i = 1; i <= 5; i++){
        printf(" <File%d>:<size%d>", i, i);
    }
    printf("\n");

    printf("$%s <IP-Address>", progname);
    for(int i = 1; i <= 5; i++){
        printf(" <File%d>:<size%d>", i, i);
    }
    printf("\n");

    printf("In the second method, the defaultport (%d) will be used.\n", DEFAULT_PORT);
    printf("At least the IP-Address and one file must be provided.\n");
}
