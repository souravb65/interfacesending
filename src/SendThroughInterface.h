/*
 * SendThroughInterface.h
 *
 *  Created on: 26-Nov-2016
 *      Author: abhijit
 */

#ifndef SENDTHROUGHINTERFACE_H_
#define SENDTHROUGHINTERFACE_H_
#include <stdio.h>
//#include <stdlib.h>
#include <unistd.h>           // close()
#include <cstring>           // strcpy, memset(), and memcpy()

#include <netdb.h>            // struct addrinfo
#include <sys/types.h>        // needed for socket(), uint8_t, uint16_t, uint32_t
#include <sys/socket.h>       // needed for socket()
#include <netinet/in.h>       // IPPROTO_RAW, IPPROTO_UDP, INET_ADDRSTRLEN
#include <netinet/ip.h>       // struct ip and IP_MAXPACKET (which is 65535)
#include <netinet/udp.h>      // struct udphdr
#include <arpa/inet.h>        // inet_pton() and inet_ntop()
#include <sys/ioctl.h>        // macro ioctl is defined
#include <bits/ioctls.h>      // defines values for argument "request" of ioctl.
#include <net/if.h>           // struct ifreq
#include <linux/if_ether.h>   // ETH_P_IP = 0x0800, ETH_P_IPV6 = 0x86DD
#include <linux/if_packet.h>  // struct sockaddr_ll (see man 7 packet)
#include <net/ethernet.h>

#include <errno.h>            // errno, perror()

// Define some constants.
#define IP4_HDRLEN 20         // IPv4 header length
#define UDP_HDRLEN  8         // UDP header length, excludes data


class SendThroughInterface {
public:
	SendThroughInterface(char *intf, struct ether_addr dst_addr, in_addr src_ip, int src_port);
	virtual ~SendThroughInterface();
	int init();
	int send(char *data, int datalen, struct in_addr dst_ip, int dst_port);
private:
	uint16_t udp4_checksum (struct ip iphdr, struct udphdr udphdr, uint8_t *payload, int payloadlen);
	uint16_t checksum (uint16_t *addr, int len);
	uint8_t ether_frame[IP_MAXPACKET];
	char interface[IF_NAMESIZE];
	struct in_addr src_ip;
	int ip_flags[4];
	struct ether_addr e_src_addr, e_dst_addr;
	struct sockaddr_ll device;
    struct ip iphdr;
    struct udphdr udphdr;
    int src_port;
    int socket_descriptor;

};

#endif /* SENDTHROUGHINTERFACE_H_ */
