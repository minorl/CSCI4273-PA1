#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <map>
#include <string>
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>

using namespace std;  

#define MAX_NAME_LEN 8
#define BUFLEN 50
int handleStart(string arg);
string sendToUDP(int fd, char buf[], struct sockaddr_in &remaddr, socklen_t &slen);
int createTCP(char* addr, const char* port);
int main(int argc, char *argv[])
{
    string command, arg, message, returnstring;
    if(argc != 3){
        printf("Wrong number of arguments. Usage %s <server hostname> <server port> \n", argv[0]);
        return 1;
    }
    /* Set up UDP socket to talk to coordinator */
    struct sockaddr_in myaddr, remaddr;
    int fd;
    socklen_t slen=sizeof(remaddr);
    char buf[BUFLEN];   /* message buffer */

    /* create a socket */
    if ((fd=socket(AF_INET, SOCK_DGRAM, 0))<0)
        perror("socket\n");

    /* bind it to all local addresses and pick any port number */
    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(0);

    if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("bind failed");
        return 0;
    }       

    /* now define remaddr, the address to whom we want to send messages */
    /* For convenience, the host address is expressed as a numeric IP address */
    /* that we will convert to a binary format via inet_aton */

    memset((char *) &remaddr, 0, sizeof(remaddr));
    remaddr.sin_family = AF_INET;
    remaddr.sin_port = htons(atoi(argv[2]));
    if (inet_aton(argv[1], &remaddr.sin_addr)==0) {
        fprintf(stderr, "inet_aton() failed\n");
        return -1;
    }

    while(true){
        memset((char *) buf, 0, sizeof(char)*BUFLEN);
        cin >> command;
        if(!command.compare("Start")){
            cin >> arg;
            strcat(buf, "Start ");
            strcat(buf, arg.c_str());
            cout << "trying to send" <<  buf << endl;
            returnstring = sendToUDP(fd, buf, remaddr, slen);
            if(returnstring.compare("-1")){
                createTCP(argv[1], returnstring.c_str());
                cout << "A new chat session " << arg << " has been created and you have joined this session." << endl;
            }
            else{
                cout << "Failed to start chat session. Please make sure session does not already exist and server is running" << endl;
            }
        }
        else if(!command.compare("Join")){
            cin >> arg;
            strcat(buf, "Find ");
            strcat(buf, arg.c_str());
            cout << "trying to send" << buf << endl;
            returnstring = sendToUDP(fd, buf, remaddr, slen);
            if(returnstring.compare("-1")){
                //join
                cout << "You have joined the chat session " << arg << "." << endl;
            }
            else{
                cout << "Failed to join chat session. Please make sure session exists and server is running" << endl;
            }
        }
        else if(!command.compare("Submit")){
            getline(cin, message);
            strcat(buf, "Submit ");
            strcat(buf, message.c_str());
            returnstring = sendToUDP(fd, buf, remaddr, slen);
        }
        else if(!command.compare("GetNext")){
            cout << "GetNext received" << endl;
            strcat(buf, "GetNext");
            returnstring = sendToUDP(fd, buf, remaddr, slen);
        }
        else if(!command.compare("GetAll")){
            cout << "GetAll received" << endl;
            strcat(buf, "GetAll");
            returnstring = sendToUDP(fd, buf, remaddr, slen);
        }
        else if(!command.compare("Leave")){
            strcat(buf, "Leave");
            returnstring = sendToUDP(fd, buf, remaddr, slen);
        }
        else if(!command.compare("Exit")){
            strcat(buf, "Exit");
            returnstring = sendToUDP(fd, buf, remaddr, slen);
        }
        else{
            cout << "Invalid command. Valid commands are: " << endl;
            cout << "Join <session name>" << endl;
            cout << "Submit <message>" << endl;
            cout << "GetNext" << endl;
            cout << "GetAll" << endl;
            cout << "Leave" << endl;
            cout << "Exit" << endl;
        }
        
    }
}

string sendToUDP(int fd, char buf[], struct sockaddr_in &remaddr, socklen_t &slen){
    int recvlen;
    if (sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&remaddr, slen)<0) {
        perror("sendto");
        exit(1);
    }
    // now receive an acknowledgement from the server 
    recvlen = recvfrom(fd, buf, BUFLEN, 0, (struct sockaddr *)&remaddr, &slen);
    if (recvlen >= 0) {
            buf[recvlen] = 0;   /* expect a printable string - terminate it */
            // printf("received message: \"%s\"\n", buf);
            return buf;
        }
    return "-1";
}
int createTCP(char* addr, const char* port){
    int sock;
    struct sockaddr_in serv; //server infos
    int echoStringLen;
    int totalBytesRcvd = 0;
    int bytesRcvd;
    char echoBuffer[32];


     //establish socket, -1 failure
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 1){
        perror("socket failed");
        return -1;
    }
    

    //set server info
    memset((char*)&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(addr);
    serv.sin_port = htons(atoi(port));

    //connect to server, checks for failure
    if(connect(sock, (struct sockaddr *) &serv, sizeof(serv))<0){
        perror("connect() failed");
        return -1;
    }

    return sock;
    // echoStringLen = strlen(argv[2]);
    // send(sock, argv[2], strlen(argv[2]), 0);

    // printf("Received: ");
    // while(totalBytesRcvd < echoStringLen)
    // {
    //     if((bytesRcvd = recv(sock, echoBuffer, 31, 0)) <= 0){
    //         printf("recv() failed");
    //         return 1;       
    //     }
    //     totalBytesRcvd += bytesRcvd;
    //     echoBuffer[bytesRcvd] = '\0';
    //     printf("%s", echoBuffer);
    // }

    // printf("\n");

    // close(sock);
    // return 0;
}

