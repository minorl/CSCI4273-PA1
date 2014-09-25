#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <sys/types.h>
#include <string>
#include <unistd.h>
#include <map>

#define MAX_NAME_LEN 8

using namespace std;

void handleStart(int servSock, struct sockaddr_in clientAddr, 
        unsigned int clientLen, string sessionName, 
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

    /* Create socket for incoming connections */
    if ((servSock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("socket() failed");
        return 1;
    }
      
    /* Construct local address structure */
    memset((char*)&controllerAddr, 0, sizeof(controllerAddr));
    controllerAddr.sin_family = AF_INET;                  /* Inet addr family */
    controllerAddr.sin_addr.s_addr = htonl(INADDR_ANY);   /* Any inc interface */
    controllerAddr.sin_port = htons(controllerPort);      /* Local port */

    controllerLen = sizeof(controllerAddr);
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
            handleStart(servSock, clientAddr, clientLen, sessionName,chatSessions);
        }
        else if(strncmp("Find", recvBuffer, 4) == 0){
            printf("Find received\n");
        }
        else if(strncmp("Terminate", recvBuffer, 9) == 0){
            printf("Terminate received\n");
        }
        else{
            printf("Invalid command\n");
        }
        
    }
    /* NOT REACHED */
}
bool nameExists(string sessionName, std::map<string, unsigned short> chatSessions){
    //Returns true if session by that name exists
    std::map<string,unsigned short>::iterator it = chatSessions.find(sessionName);
    return it != chatSessions.end();
}
void handleStart(int servSock, struct sockaddr_in clientAddr, unsigned int clientLen,
        string sessionName, std::map<string, unsigned short> &chatSessions){

    const char failure[4] = "-1\0";

    printf("%s\n", sessionName.c_str());
    if(nameExists(sessionName, chatSessions)){
        if(sendto(servSock, failure, 4, 0, 
            (struct sockaddr *)&clientAddr, clientLen)<0){
            perror("sendto()");
        }
        return;
    }
    //probably should move communication stuff to outside, return value
    //create chat session
    //create tcp socket
    // chatSessions[sessionName]

}

//nc -4u -w1 localhost <port> (test udp socket)