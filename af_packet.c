#include <stdio.h>
#include <stdint.h>
#include <error.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include "af_packet.h"


int af_packet_sock_sample(const char *ifname)
{
    int sockfd;
    struct sockaddr_ll addr;
    uint8_t buffer[BUFFER_SIZE];

    sockfd = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_ALL));
    if(sockfd < 0)
    {
        perror("socket create failed\n");
        return -1;
    }



}