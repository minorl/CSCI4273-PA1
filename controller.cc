#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <sys/types.h>
#include <string>
#include <unistd.h>
#include <map>

#define MAX_NAME_LEN 8

using namespace std;

int handleStart(string sessionName, 
        std::map<string, unsigned short> &chatSessions);
bool checkName(string sessionName, std::map<string, unsigned short> chatSessions);

int main(int argc, char *argv[])
{
    int servSock;                         /* Socket descriptor for server */
    struct sockaddr_in controllerAddr;    /* Local address */
    struct sockaddr_in clientAddr;        /* Client address */
    unsigned short controllerPort = 0;    /* Server port, 0 for any available */
    unsigned int clientLen;         /* Len of client address data structure */
    socklen_t controllerLen;        /* Len of controller address data structure */
    size_t recvBufLen = 16;
    char recvBuffer[recvBufLen];          /* Buffer for command string */
    int recvMsgSize;                      /* Size of received message */
    std::map<string, unsigned short> chatSessions;
    char sessionName[8];
    int res;
    char resultString[33];

    /* Create socket for incoming connections */
    if ((servSock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("socket() failed");
        return 1;
    }
      
    /* Construct local address structure */
    controllerLen = sizeof(controllerAddr);
    memset((char*)&controllerAddr, 0, controllerLen);
    controllerAddr.sin_family = AF_INET;                  /* Inet addr family */
    controllerAddr.sin_addr.s_addr = htonl(INADDR_ANY);   /* Any inc interface */
    controllerAddr.sin_port = htons(controllerPort);      /* Local port */

    /* Bind to the local address */
    if (bind(servSock, 
             (struct sockaddr *) &controllerAddr, controllerLen) < 0){
        perror("bind() failed");
        return 1;
    }

    /* Print the port to stdout */
    if (getsockname(servSock, 
                    (struct sockaddr *)&controllerAddr, &controllerLen) < 0){
        perror("getsockname() failed");
        return 1;
    }
    printf("port number %d\n", ntohs(controllerAddr.sin_port));



    for (;;) /* Cthuloops */
    {
        /* Set the size of the in-out parameter */
        clientLen = sizeof(clientAddr);

        /* Wait for a client to connect */
        if ((recvMsgSize = recvfrom(servSock, recvBuffer, recvBufLen, 0, 
                            (struct sockaddr *) &clientAddr, &clientLen)) < 0){
            perror("recvfrom() failed");
            return 1;
        }
        /* If you like it, you should put a terminator on it */
        if(recvMsgSize > 0)
            recvBuffer[recvMsgSize] = '\0';

        /* Connected to a client */
        printf("Handling client %s\n", inet_ntoa(clientAddr.sin_addr));
        
        if(strncmp("Start", recvBuffer, 5) == 0){
            printf("Start received\n");
            strcpy(sessionName, &recvBuffer[6]);
            res = handleStart(sessionName,chatSessions);
        }
        else if(strncmp("Find", recvBuffer, 4) == 0){
            printf("Find received\n");
        }
        else if(strncmp("Terminate", recvBuffer, 9) == 0){
            printf("Terminate received\n");
        }
        else{
            printf("Invalid command\n");
            res = -1;
        }
        
        /* Send result of operation to client */
        sprintf(resultString,"%d",res);
        if(sendto(servSock, resultString, strlen(resultString), 0, 
                (struct sockaddr *)&clientAddr, clientLen)<0){
            perror("sendto() failed");
            return 1;
        }

    }
    /* NOT REACHED */
}
bool nameExists(string sessionName, std::map<string, unsigned short> chatSessions){
    //Returns true if session by that name exists
    std::map<string,unsigned short>::iterator it = chatSessions.find(sessionName);
    return it != chatSessions.end();
}
int handleStart(string sessionName, 
    std::map<string, unsigned short> &chatSessions){

    int servSock;                    /* Socket descriptor for server */
    struct sockaddr_in servAddr;     /* Local address */
    socklen_t servLen = sizeof(servAddr); /* Len of serv address data structure */
    unsigned short servPort = 0;     /* Any port */


    /* Check to see if session name is valid */
    printf("%s\n", sessionName.c_str());
    if(sessionName.empty() || nameExists(sessionName, chatSessions)){
        return -1;
    }

    /* Create tcp socket for incoming connections */
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        printf("socket() failed");
        return 1;
    }

    memset((char*)&servAddr, 0, servLen);
    servAddr.sin_family = AF_INET;                /* Internet address family */
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    servAddr.sin_port = htons(servPort);                 /* Any port */

        /* Bind to the local address */
    if (bind(servSock, (struct sockaddr *) &servAddr, servLen) < 0){
        perror("bind() failed");
        return -1;
    }
        /* Mark the socket so it will listen for incoming connections */
    if (listen(servSock, 5) < 0){
        perror("listen() failed");
        return -1;
    }

    /* Print the port to stdout */
    if (getsockname(servSock, 
                    (struct sockaddr *)&servAddr, &servLen) < 0){
        perror("getsockname() failed");
        return -1;
    }
    servPort = servAddr.sin_port;

    //create chat session
     //exec sessionServer

    chatSessions[sessionName.c_str()] = servPort;
    return servPort;
}

//nc -4u -w1 localhost <port> (test udp socket)