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

      memset(buf, 0, MAXBUFLEN);

      addr_len = sizeof their_addr;
      if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
              (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
      }

      Window window;
      int status = window.disassemble(buf);
      if (status < 0) {
        printf("Error requesting file %s.  Please try again.\n", buf);
      }

      // ChangeThis: Implement window (queue) to send the files
      queue<Packet*> sliding_window;
      int window_position = 0;

      int packetsLeft = window.packets.size();
      int i;
      
      // Loop while there are still files to be sent
      while (packetsLeft != 0) {


        //printf("Queue has %d items \n", sliding_window.size());
        // While the queue has less than 5 elements in it
        while (sliding_window.size() < SLIDINGWINDOWSIZE) {

          //printf("Entered loop for the %d'th time\n", i);
          // Set the timer
          window.packets[i]->header.setTimestamp(time(&timer));

          sliding_window.push(window.packets[i]);

          sendto(sockfd, window.packets[i], sizeof(Packet), 0,
            (struct sockaddr *)&their_addr, addr_len);

          packetsLeft--;
          i++;
          
        }

        Packet *ack_packet = new Packet();
        recvfrom(sockfd, ack_packet, sizeof(Packet), 0 , NULL, 0); //code wont move on unless client recieved something. expecting an ack

        //pop queue for all ack numbers received in order

        int diff = ((ack_packet->header.getAckNum() - sliding_window.front()->header.getSeqNum()) / 1024);

        while (diff >= 0) {

              printf("Ack number is: %d\n", ack_packet->header.getAckNum());
              sliding_window.pop();
              window_position++; //new slot has opened up in the window
              diff--; //we need to pop the queue "diff" many times for multiple packet acks
        }
        
      }

      /*Get remaining acks after termination of while loop. Code is a re-write : bad, might be able to clean up */

      while(!sliding_window.empty())
      {
        Packet *ack_packet = new Packet();
        recvfrom(sockfd, ack_packet, sizeof(Packet), 0 , NULL, 0);

        int diff = (ack_packet->header.getAckNum() - sliding_window.front()->header.getSeqNum()) / 1024;

        while (diff >= 0) {

              printf("Ack number is: %d\n", ack_packet->header.getAckNum());
              sliding_window.pop();
              window_position++; //new slot has opened up in the window
              diff--; //we need to pop the queue "diff" many times for multiple packet acks
        }

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
