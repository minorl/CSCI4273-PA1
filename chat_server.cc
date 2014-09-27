#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <sys/types.h>
#include <string>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <map>
#include <string.h>

#define MAXMSG 128

int read_from_client(int filedes);

int main(int argc, char *argv[]){
	int servSock, clientSock;
	struct sockaddr_in clientAddr;        /* Client address */
	unsigned int clientLen;         /* Len of client address data structure */
	fd_set active_fd_set, read_fd_set;
	printf("LOOK MA I STARTED\n");

	if (argc != 2)     /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage:  %s <Server Sock FD>\n", argv[0]);
        return 1;
    }

    printf("passin checks");
    /* Set the size of the in-out parameter */
    clientLen = sizeof(clientAddr);

    /* Parse input argument */
    servSock = atoi(argv[1]);
    printf("this is my serv sock: %d", servSock);
    /* Mark the socket so it will listen for incoming connections */
	if (listen(servSock, 5) < 0){
	        perror("listen() failed");
	        return -1;
	    }

  	/* Initialize the set of active sockets. */
	FD_ZERO (&active_fd_set);
  FD_SET (servSock, &active_fd_set);

  	printf("BOUT TO LOOP BITCH");
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
              }
            else
              {
                /* Data arriving on an already-connected socket. */
                if (read_from_client (i) < 0)
                  {
                    close (i);
                    FD_CLR (i, &active_fd_set);
                  }
              }
          }
    }
}

int read_from_client (int filedes)
{
  char buffer[MAXMSG];
  int nbytes;

  nbytes = read (filedes, buffer, MAXMSG);
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
      return 0;
    }
}



