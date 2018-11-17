#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/wait.h>

#include <netinet/in.h>
#include <netinet/ip.h> 
#include <netdb.h>
#include <sys/socket.h>

#include <signal.h>


#include "worker.h"
#include "server.h"




static struct sigaction actINT, actCHLD;
static socket_t serverSocket, clientSocket;

int main(int argc, char const *argv[]){

    serverSocket = -1;
    clientSocket = -1;

    // Setting port
    u_int16_t portNumber;
    if(argc < 2){
        portNumber = DEFAULT_PORT;
    } else {
        portNumber = atoi(argv[1]);
    }


    // Setup
    installSigHandlers();
    printf("Starting server on port %ld, to shut down press CTRL+C\n", portNumber);

    serverSocket = createSocket(portNumber, DEFAULT_QUEUE_SIZE);
    printf("Listening...\n");


    // Main loop
    struct sockaddr_in clientAddress;
    int pid;

    while(TRUE){
        int s = sizeof(clientAddress);
        clientSocket = accept(serverSocket, (struct sockaddr*) &clientAddress, &s);
        if(clientSocket == -1){
            perror("Could not accept client");
            continue;
        }
        printf("Connection accepted\n");
        fflush(stdout);

        pid = fork();

        if(pid < 0){
            // Failure
            perror("Error while creating childprocess.\n");
        } else if(pid == 0){
            // Child
            close(serverSocket);
            startWorker(clientSocket);
            close(clientSocket);
            exit(0);
        } else { 
            // Father
            printf("Processing in Process %d\n", pid);
        }
        close(clientSocket);
        clientSocket = -1;
    }

    return 0;
}

socket_t createSocket(u_int16_t portno, size_t queueSize){

    // Recieving socket handle
    socket_t s = socket(AF_INET, SOCK_STREAM, 0);
    if(s < 0){
        perror("Error while creating socket");
        exit(1);
    }


    // binding to port
    struct sockaddr_in serverAddress;
    bzero((char*) &serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(portno);
    int r = bind(s, (struct sockaddr*) &serverAddress, sizeof(serverAddress));
    if(r < 0){
        perror("Error while binding");
        exit(2);
    }

    // set passive
    r = listen(s, queueSize);
    if(r < 0){
        perror("Error while setting to passive mode");
        exit(3);
    }

    return s;
}

void sigIntHandle(int signum, siginfo_t *info, void *ptr){
    // close open sockets
    printf("\nShutting down server\n");
    close(serverSocket);
    if(clientSocket != -1){
        close(clientSocket);
    }
    exit(0);
}

void sigChldHandle(int signum, siginfo_t *info, void *ptr){
    // Release zombie process
    int stat;
    waitpid(info->si_pid, &stat, 0);
}

void installSigHandlers(){
    
    // SIGINT
    memset(&actINT, 0, sizeof(actINT));

    actINT.sa_sigaction = sigIntHandle;
    actINT.sa_flags = SA_SIGINFO;

    sigaction(SIGINT, &actINT, NULL);


    // SIGCHLD
    memset(&actINT, 0, sizeof(actCHLD));

    actCHLD.sa_sigaction = sigChldHandle;
    actCHLD.sa_flags = SA_RESTART;

    sigaction(SIGCHLD, &actCHLD, NULL);
}
