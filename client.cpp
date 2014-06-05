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
// #define PROBABILITY 1 // 0.1 probability
#define EXPIRY 1

void sendACK(int ack_num, int sockfd, struct addrinfo *p) {
    Packet *ack_packet = new Packet();
    ack_packet->header.setAckNum(ack_num);

    sendto(sockfd, ack_packet, sizeof(ack_packet), 0, p->ai_addr, p->ai_addrlen);
}

bool simulatePacketLoss(int prob)
{
    int random_number = rand() % 100 + 1; //random number between 1 - 100;

    return (random_number <= prob);
}

bool simulatePacketCorruption(int prob) {
    int random_number = rand() % 100 + 1; //random number between 1 - 100;

    return (random_number <= prob);
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv; int rv_timer;
    int numbytes;
    int last_ack_number = 0;
    fd_set readfds;
    struct timeval tv;

    srand (time(NULL));

    if (argc != 5) {
        fprintf(stderr,"usage: ./client hostname filename prob_loss prob_corruption\n");
        exit(1);
    }

    // Set prob_loss and prob_corruption as variables
    int prob_loss = atoi(argv[3]);
    int prob_corruption = atoi(argv[4]);

    if (prob_loss > 100 || prob_loss < 0 || prob_corruption > 100 || prob_corruption < 0) {
        fprintf(stderr,"probabilities need to be between 0 and 100\n");
        exit(1);
    }

    // printf("%d", prob_loss);
    // printf("%d", prob_corruption);

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
    bool flag = false;


    while(1) {


    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);

        if(num_received != 0 && (FD_ISSET(sockfd, &readfds)) ) 
        {
            tv.tv_sec = 1;
            tv.tv_usec =  0;
            rv_timer = select(sockfd + 1, &readfds, NULL, NULL, &tv);

            if (rv_timer == -1) {
               
                perror("select"); // error occurred in select()

            } else if (rv_timer == 0) {

                printf("Timeout occured for ack %d\n", last_ack_number);
                sendACK(last_ack_number, sockfd, p);
                continue;
            }
        }

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

            // Simulate Packet Loss
            if (simulatePacketLoss(prob_loss)) {
                printf("Dropped packet: %d (simulated). \n", packet->header.getSeqNum());
                flag = true;
                delete packet;
                continue;
            }

            // Simulate Packet Corruption
            if (simulatePacketCorruption(prob_corruption)) {
                printf("Packet corrupted: %d (simulated). \n", packet->header.getSeqNum());
                
                // Send the ACK of the last received packet.
                last_ack_number = 0;
                sendACK(last_ack_number, sockfd, p);
                delete packet;
                continue;
            }

            if (packet->header.getSeqNum() > 1024) {
              continue;
            }

            window.packets.push_back(packet);
            last_ack_number = packet->header.getSeqNum();
            sendACK(last_ack_number, sockfd, p);
            printf("Received First Packet:%d\n", last_ack_number);
            num_received++;

        } else {
            // Get sizes
            int prev_seq_num = window.packets.back()->header.getSeqNum();
            int curr_packet_seq_num = packet->header.getSeqNum();
            int curr_packet_size = packet->header.getContentLength();

            if (curr_packet_seq_num - prev_seq_num == curr_packet_size) {

                // Simulate Packet Loss
                if (simulatePacketLoss(prob_loss)) {
                    printf("Dropped packet: %d (simulated). \n", packet->header.getSeqNum());
                    delete packet;
                    continue;
                }

                // Simulate Packet Corruption
                if (simulatePacketCorruption(prob_corruption)) {
                    printf("Packet corrupted: %d (simulated). \n", packet->header.getSeqNum());


                    // Send the ACK of the last received packet.
                    last_ack_number = prev_seq_num;
                    //printf("PrevSeqNum: %d\n", last_ack_number);
                    sendACK(prev_seq_num, sockfd, p);
                    delete packet;
                    continue;
                }

                // Packets in order
                //printf("window.packetsize: %zu\n", window.packets.size());
                window.packets.push_back(packet);
                printf("Received Packet in Order\n");
                printf("Packet number is: %d\n", curr_packet_seq_num);
                last_ack_number = packet->header.getSeqNum();
                sendACK(last_ack_number, sockfd, p);
                num_received++;
            } else {
                // Else, drop the packet
                printf("Received Packet out of order. Dropped. \n"); //Packet Loss, might have to fix this. 
                // send ACK of last previously received packet (in order)
                last_ack_number = prev_seq_num;
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
