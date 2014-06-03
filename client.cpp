/*
** client.c
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
#include <time.h>

#include "Window.h"

#define SERVERPORT "4951"    // the port users will be connecting to
#define PROBABILITY 1 // 0.1 probability

void sendACK(int ack_num, int sockfd, struct addrinfo *p) {
    Packet *ack_packet = new Packet();
    ack_packet->header.setAckNum(ack_num);

    sendto(sockfd, ack_packet, sizeof(ack_packet), 0, p->ai_addr, p->ai_addrlen);
}

bool simulatePacketLoss()
{
    int prob = rand() % 10 + 1; //numbers between 1 - 10;

    return (prob <= PROBABILITY);
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;

    srand (time(NULL));

    if (argc != 3) {
        fprintf(stderr,"usage: talker hostname filename\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to bind socket\n");
        return 2;
    }

    if ((numbytes = sendto(sockfd, argv[2], strlen(argv[2]), 0,
             p->ai_addr, p->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }

    Window window;
    int num_received = 0;

    // REMOVEME
    bool asdf = false;


    while(1) {
        //printf("Waiting to recieve\n");
        Packet *packet = new Packet();
        //printf("Size of packet: %d\n", sizeof(Packet));
        recvfrom(sockfd, packet, sizeof(Packet), 0 , NULL, 0);
        if (packet->header.getFin() == 1) {
            break;
        }
        // Check to see if packet is received in order
        // If in order, push back into window
        if (num_received == 0) {
            window.packets.push_back(packet);
            sendACK(packet->header.getSeqNum(), sockfd, p);
            printf("Received First Packet.\n");
            num_received++;
        }
        else {
            // Get sizes
            int prev_seq_num = window.packets.back()->header.getSeqNum();
            int curr_packet_seq_num = packet->header.getSeqNum();
            int curr_packet_size = packet->header.getContentLength();

            if (curr_packet_seq_num - prev_seq_num == curr_packet_size) {

                if (simulatePacketLoss && !asdf) {
                    // drop a bitch
                    printf("Dropped packet (simulated)");
                    asdf = true;
                    continue;
                }

                // Packets in order
                window.packets.push_back(packet);
                printf("Received Packet in Order\n");
                printf("Packet number is: %d\n", curr_packet_seq_num);
                sendACK(packet->header.getSeqNum(), sockfd, p);
                num_received++;
            } else {
                // Else, drop the packet
                printf("Received Packet out of order. Dropped. \n"); //Packet Loss, might have to fix this. 
               // TODO: Resend ACK - DONE
                // sendACK of last previously received packet (in order)
                sendACK(prev_seq_num, sockfd, p);
                
            }
        }
    }

    window.assemble(argv[2]);

    freeaddrinfo(servinfo);

    printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);
    close(sockfd);

    return 0;
}
