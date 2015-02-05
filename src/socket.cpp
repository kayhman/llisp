/* 
 * tcpserver.c - A simple TCP echo server 
 * usage: tcpserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>


#define BUFSIZE 1024

#if 0
/* 
 * Structs exported from in.h
 */

/* Internet address */
struct in_addr {
  unsigned int s_addr; 
};

/* Internet style socket address */
struct sockaddr_in  {
  unsigned short int sin_family; /* Address family */
  unsigned short int sin_port;   /* Port number */
  struct in_addr sin_addr;	 /* IP address */
  unsigned char sin_zero[...];   /* Pad to size of 'struct sockaddr' */
};

/*
 * Struct exported from netdb.h
 */

/* Domain name service (DNS) host entry */
struct hostent {
  char    *h_name;        /* official name of host */
  char    **h_aliases;    /* alias list */
  int     h_addrtype;     /* host address type */
  int     h_length;       /* length of address */
  char    **h_addr_list;  /* list of addresses */
}
#endif

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}


int lsocket(const std::string& domain, const std::string& type, const std::string& protocol)
{
  const int parentfd = socket(AF_INET, SOCK_STREAM, 0);
  int optval; /* flag value for setsockopt */
  if (parentfd < 0) 
    error((char*)"ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  optval = 1;
  setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));

  return parentfd;
}

int lbind(const int parentfd,  const int portno)
{
  struct sockaddr_in serveraddr; /* server's addr */
  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));

  /* this is an Internet address */
  serveraddr.sin_family = AF_INET;

  /* let the system figure out our IP address */
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

  /* this is the port we will listen on */
  serveraddr.sin_port = htons((unsigned short)portno);

  /* 
   * bind: associate the parent socket with a port 
   */
  if (bind(parentfd, (struct sockaddr *) &serveraddr,
	   sizeof(serveraddr)) < 0)
    error((char*)"ERROR on binding");

  return 0;
}

int llisten(const int parentfd, const int queueSize)
{
   /* 
   * listen: make this socket ready to accept connection requests 
   */
  if (listen(parentfd, queueSize) < 0) /* allow 5 requests to queue up */ 
    error((char*)"ERROR on listen");

  return 0;
}

int laccept(const int parentfd, struct sockaddr* clientaddr,  const int clientlen)
{
    /* 
     * accept: wait for a connection request 
     */
    int childfd = accept(parentfd, clientaddr, (socklen_t*)&clientlen);
    if (childfd < 0) 
      error((char*)"ERROR on accept");

    return childfd;
}


int main(int argc, char **argv) {
  int parentfd; /* parent socket */
  int childfd; /* child socket */
  int clientlen; /* byte size of client's address */
  int portno;
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buffer */
  char *hostaddrp; /* dotted decimal host addr string */
  int n; /* message byte size */

  /* 
   * check command line arguments 
   */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);

  /* 
   * socket: create the parent socket 
   */
  parentfd = lsocket("AF_INET", "SOCK_STREAM", "0");

  lbind(parentfd, portno);

  llisten(parentfd, 5);
 
  /* 
   * main loop: wait for a connection request, echo input line, 
   * then close connection.
   */
  clientlen = sizeof(clientaddr);
  while (1) {

    childfd = laccept(parentfd, (struct sockaddr*)&clientaddr, clientlen);
    
    /* 
     * gethostbyaddr: determine who sent the message 
     */
    hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
			  sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
      error((char*)"ERROR on gethostbyaddr");
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
      error((char*)"ERROR on inet_ntoa\n");
    printf("server established connection with %s (%s)\n", 
	   hostp->h_name, hostaddrp);
    
    /* 
     * read: read input string from the client
     */
    bzero(buf, BUFSIZE);
    n = read(childfd, buf, BUFSIZE);
    if (n < 0) 
      error((char*)"ERROR reading from socket");
    printf("server received %d bytes: %s", n, buf);
    
    /* 
     * write: echo the input string back to the client 
     */
    n = write(childfd, buf, strlen(buf));
    if (n < 0) 
      error((char*)"ERROR writing to socket");

    close(childfd);
  }
}
