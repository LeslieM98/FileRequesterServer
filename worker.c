#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h> 
#include <netinet/ip.h> 
#include <arpa/inet.h>


#include "worker.h"
#include "workerLib.h"



static pid_t currentPid;

void startWorker(socket_t socketId){

    currentPid = getpid();
    char *clientIp = getClientIP(socketId);


    // Important request informations extracted from incoming package
    u_int16_t nBytes;
    u_int16_t pathLength;
    char *path = NULL;


    nBytes = getRequestedNBytes(socketId);
    if(nBytes == 0){
        printf("%d: %s requested %d bytes, request ignored\n", currentPid, clientIp, nBytes);
        return;
    }

    pathLength = getFilePathLength(socketId);

    path = getFilePath(socketId, pathLength);

    printf("%d: %s requested %d bytes of: %s\n", currentPid, clientIp, nBytes, path);
    fflush(stdout);



    // Important outgoing information written to the return package
    u_int8_t status = 0;
    u_int16_t contentSize;
    u_int8_t *content = NULL;
    u_int8_t *package = NULL;


    // Get file content
    content = initBuffer(nBytes);
    contentSize = getFileContent(path, content, nBytes);
    
    /*
     * Determine statusbyte
     * 
     * 0 - Returned content size == requested content size
     * 1 - Returned content size == 0
     * 2 - Returned content size < requested contentsize 
     */
    if(contentSize == 0){
        status = 1;
    } else if (contentSize != nBytes){
        status = 2;
    }


    // Outgoing package
    package = initBuffer(OUT_OVERHEAD + contentSize);
    preparePackage(status, contentSize, content, package);

    returnPackage(package, OUT_OVERHEAD + contentSize, socketId);


    // Cleanup
    free(package);
    free(content);
    free(clientIp);
}

u_int8_t *initBuffer(size_t size){
    size_t byteCount = size * sizeof(int8_t);

    u_int8_t *tmp = malloc(byteCount);
    memset(tmp, 0, byteCount);

    return tmp;
}

size_t getFileContent(char *filename, void *buffer, size_t size){
    size_t readBytes;
    FILE *requestedFile = fopen(filename, "r");
    if(!requestedFile){
        perror("Error while opening file");
        return 0;
    }
    readBytes = fread(buffer, 1, size, requestedFile);
    fclose(requestedFile);

    return readBytes;
}

char *getClientIP(socket_t socketfd){
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int res = getpeername(socketfd, (struct sockaddr *)&addr, &addr_size);
    char *clientip = malloc(sizeof(char[20]));
    strcpy(clientip, inet_ntoa(addr.sin_addr));
    return clientip;
}

u_int16_t getRequestedNBytes(socket_t requestSocket){
    u_int16_t nbytes;
    read(requestSocket, &nbytes, REQ_BYTES);

    return nbytes;
}

u_int16_t getFilePathLength(socket_t requestSocket){
    u_int16_t *buff = (u_int16_t*) initBuffer(FILE_BYTES);
    u_int16_t filePathLength;
    read(requestSocket, &filePathLength, FILE_BYTES);
    free(buff);

    return filePathLength;
}

char *getFilePath(socket_t requestSocket, u_int16_t pathLength){
    char *buff = initBuffer(pathLength + 1);
    read(requestSocket, buff, pathLength);
    return buff;
}

void preparePackage(u_int8_t status, u_int16_t contentSize, u_int8_t *content, u_int8_t *dest){
    dest[0] = status;
    *((u_int16_t*) &dest[STAT_BYTES]) = contentSize;
    for(int i = 0; i < contentSize; i++){
        dest[OUT_OVERHEAD + i] = content[i];
    }

}

void returnPackage(u_int8_t *package, size_t packageSize, socket_t socket){
    if(write(socket, package, packageSize) < 0){
        printf("%d: ", currentPid);
        perror("Error while returning package");
    }
}
