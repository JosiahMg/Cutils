#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include "af_packet.h"


int af_packet_sock_sample(const char *ifname)
{
    int sockfd;
    struct sockaddr_ll addr;
    uint8_t buffer[BUFFER_SIZE];

    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if(sockfd < 0)
    {
        perror("socket create failed\n");
        return -1;
    }

    memset(&addr, 0x00, sizeof(addr));
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(ETH_P_ALL);
    addr.sll_ifindex = if_nametoindex(ifname);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind failed\n");
        return -1;
    }

    int val = 1;

    if (setsockopt(sockfd, SOL_PACKET, PACKET_AUXDATA, &val, sizeof(val)) == -1 && errno != ENOPROTOOPT)
    {
        close(sockfd);
    }

	union {
		struct cmsghdr	cmsg;
		char		buf[CMSG_SPACE(sizeof(struct tpacket_auxdata))];
	} cmsg_buf;
	struct iovec	iov;
	struct msghdr	msg;
    struct sockaddr_ll addr_in;

	memset(&cmsg_buf, 0x00, sizeof(cmsg_buf));
	memset(&iov, 0x00, sizeof(iov));
	memset(&msg, 0x00, sizeof(msg));

	iov.iov_base = buffer;
	iov.iov_len = BUFFER_SIZE;

	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

    int addr_len = sizeof(addr_in);
	msg.msg_name = &addr_in;
	msg.msg_namelen = addr_len;

	msg.msg_control = &cmsg_buf;
	msg.msg_controllen = sizeof(cmsg_buf);


    while(1)
    {
        int data_size = recvmsg(sockfd, &msg, 0);
        // int data_size = recvmsg(sockfd, &msg, MSG_DONTWAIT);
        //int data_size = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&addr_in, (socklen_t *)&addr_len);
        if (data_size < 0)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                perror("Recvfrom failed\n");
                close(sockfd);
                return -1;
            }
            continue;
        }
        //解析msg数据 查看收到的报文是否带有vlan属性
        for (struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg)) 
        {
		    if (cmsg->cmsg_len >= CMSG_LEN(sizeof(struct tpacket_auxdata)) &&
                cmsg->cmsg_level == SOL_PACKET &&
                cmsg->cmsg_type == PACKET_AUXDATA) 
            {
                struct tpacket_auxdata *aux = (struct tpacket_auxdata *) CMSG_DATA(cmsg);

                if (aux && (aux->tp_status & TP_STATUS_VLAN_VALID))
                {
                    printf("pkt vlan=%d\n", aux->tp_vlan_tci & 0xfff);
                }
                break;
            }
		}
        printf("Recv data len=%d\n", data_size);
	}
    close(sockfd);
    return 0;
}