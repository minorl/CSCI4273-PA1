#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <sys/types.h>
#include <string>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

using namespace std;
#define MAXMSG 128
#define MAXCLIENTS 50


int read_from_client(int filedes, int message_number[], vector<string> &messages);
void handleGetNext(int filedes, int message_number[], vector<string> &messages);
void handleGetAll(int filedes, int message_number[], vector<string> &messages);

int main(int argc, char *argv[]){
	int servSock, clientSock;
	struct sockaddr_in clientAddr;        /* Client address */
	unsigned int clientLen;         /* Len of client address data structure */
	fd_set active_fd_set, read_fd_set;
  std::vector<string> messages;
  int message_number[MAXCLIENTS]; /* Index by client fd, current message read */

	if (argc != 2)     /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage:  %s <Server Sock FD>\n", argv[0]);
        return 1;
    }


    /* Set the size of the in-out parameter */
    clientLen = sizeof(clientAddr);

    /* Parse input argument */
    servSock = atoi(argv[1]);

    /* Mark the socket so it will listen for incoming connections */
    if (listen(servSock, 5) < 0){
        perror("listen() failed");
        return -1;
    }

  	/* Initialize the set of active sockets. */
	  FD_ZERO (&active_fd_set);
    FD_SET (servSock, &active_fd_set);


	 while (1){
      /* Block until input arrives on one or more active sockets. */
      read_fd_set = active_fd_set;
      if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0)
        {
          perror ("select");
          exit (EXIT_FAILURE);
        }

      /* Service all the sockets with input pending. */
      for (int i = 0; i < FD_SETSIZE; ++i)
        if (FD_ISSET (i, &read_fd_set))
          {
            if (i == servSock)
              {
                /* Connection request on original socket. */
                clientSock = accept(servSock, (struct sockaddr *)&clientAddr, 
                                &clientLen);
                if (clientSock < 0)
                  {
                    perror ("accept");
                    exit (EXIT_FAILURE);
                  }
                fprintf (stderr,
                         "Server: connect from host %s, port %hd.\n",
                         inet_ntoa (clientAddr.sin_addr),
                         ntohs (clientAddr.sin_port));
                FD_SET (clientSock, &active_fd_set);
                message_number[clientSock] = 0;
              }
            else
              {
                /* Data arriving on an already-connected socket. */
                if (read_from_client (i, message_number, messages) < 0)
                  {
                    close (i);
                    FD_CLR (i, &active_fd_set);
                  }
              }
          }
    }
}

int read_from_client (int filedes, int message_number[], vector<string> &messages)
{
  char buffer[MAXMSG];
  int nbytes, j;
  memset( buffer, '\0', sizeof(char)*MAXMSG);

  nbytes = read(filedes, buffer, MAXMSG);
  if (nbytes < 0)
    {
      /* Read error. */
      perror ("read");
      exit (EXIT_FAILURE);
    }
  else if (nbytes == 0)
    /* End-of-file. */
    return -1;
  else
    {
      /* Data read. */
      fprintf (stderr, "Server: got message: `%s'\n", buffer);
        if(strncmp("Submit", buffer, 6) == 0){
            printf("Submit received\n");
            j = 7;
            while(j < nbytes && buffer[j] != ' '){
              j++;
            }
            j++;
            messages.push_back(&buffer[j]);
            // printf("%s\n", messages.back().c_str());

        }
        else if(strncmp("GetNext", buffer, 7) == 0){
            printf("GetNext received, trying to handle\n");
            handleGetNext(filedes, message_number, messages);
            printf("handled?\n");
        }
        else if(strncmp("GetAll", buffer, 6) == 0){
            printf("GetAll received\n");
            handleGetAll(filedes, message_number, messages);
        }
        else if(strncmp("Leave", buffer, 5) == 0){
            printf("Leave received\n");
            return -1; //closes socket in main
        }
        else{
            printf("Invalid command\n");
        }
      return 0;
    }
}

void handleGetNext(int client, int message_number[], vector<string> &messages){
  if(message_number[client] >= messages.size()){
    return;
  }
  int nMessageToWrite = message_number[client];
  string message = messages[nMessageToWrite];
  if (send(client, message.c_str(), message.length(), 0) != message.length()){
            printf("send() failed");
  }
  message_number[client]++;
}

void handleGetAll(int client, int message_number[], vector<string> &messages){
  while(message_number[client] < messages.size()){
    handleGetNext(client, message_number, messages);
  }
}


