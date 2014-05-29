/*
** server.c 
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <queue>
#include <time.h>

#include "Window.h"

#define MYPORT "4951"    // the port users will be connecting to

#define MAXBUFLEN 100
#define SLIDINGWINDOWSIZE 5

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    // Used to get timestamps
    time_t timer;

    while(1) {
      printf("listener: waiting to recvfrom...\n");

      addr_len = sizeof their_addr;
      if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
              (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
      }

      Window window;
      window.disassemble(buf);

      // ChangeThis: Implement window (queue) to send the files
      queue<Packet> sliding_window;
      int window_position = 0;

      int packetsLeft = window.packets.size();
      int i = 0;
      // Loop while there are still files to be sent
      while (packetsLeft != 0) {

        // TODO: Pop queue when ACK for first one is received
        if () {
          sliding_window.pop();
          window_position++;
        }

        // While time queue has less than 5 elements in it
        while (sliding_window.size() < SLIDINGWINDOWSIZE) {
          // Set the timer
          window.packets[i]->header.setTimeStamp(time(&timer));

          sliding_window.push(window.packets[i]);

          sendto(sockfd, window.packets[i], sizeof(Packet), 0,
            (struct sockaddr *)&their_addr, addr_len);

          packetsLeft--;
          
          i++;
        }

        
        printf("Packet Sent\n");
      }

      Packet *fin = new Packet();
      fin->header.setFin(1);
      sendto(sockfd, fin, sizeof(Packet), 0,
          (struct sockaddr *)&their_addr, addr_len);

      printf("listener: got packet from %s\n",
          inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s));
      printf("listener: packet is %d bytes long\n", numbytes);
      buf[numbytes] = '\0';
      printf("listener: packet contains \"%s\"\n", buf);

    }
    close(sockfd);

    return 0;
}
