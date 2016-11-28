/*
 * SendThroughInterface.cpp
 *
 *  Created on: 26-Nov-2016
 *      Author: abhijit
 */

#include "SendThroughInterface.h"
#include <sstream>
#include <iostream>
SendThroughInterface::SendThroughInterface(char *intf, struct ether_addr dst_addr, in_addr src_ip, int src_port):
	device(), src_port(src_port), socket_descriptor(0), e_dst_addr(dst_addr), src_ip(src_ip) {
	strcpy(interface, intf);
}

SendThroughInterface::~SendThroughInterface() {
}

int SendThroughInterface::init(){

	int sd; //socket descriptor

	// Submit request for a socket descriptor to look up interface.
	if ((sd = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
		perror ("socket() failed to get socket descriptor for using ioctl() ");
		return -1;
	}
	struct ifreq ifr;
	std::strcpy(ifr.ifr_ifrn.ifrn_name, interface);

	if (ioctl (sd, SIOCGIFHWADDR, &ifr) < 0) {
		perror ("ioctl() failed to get source MAC address ");
		return 1;
	}
	close (sd);

    // Copy source MAC address.
    std::memcpy (e_src_addr.ether_addr_octet, ifr.ifr_hwaddr.sa_data, 6);

    // Find interface index from interface name and store index in
    // struct sockaddr_ll device, which will be used as an argument of sendto().
    if ((device.sll_ifindex = if_nametoindex (interface)) == 0) {
        perror ("if_nametoindex() failed to obtain interface index ");
        return 2;
    }

    device.sll_family = AF_PACKET;
    device.sll_protocol = htons (ETH_P_IP);
    std::memcpy (device.sll_addr, e_dst_addr.ether_addr_octet, 6);
    device.sll_halen = 6;


    //lets initializes ip hdr
    iphdr.ip_hl = IP4_HDRLEN / sizeof (uint32_t);

    // Internet Protocol version (4 bits): IPv4
    iphdr.ip_v = 4;

    // Type of service (8 bits)
    iphdr.ip_tos = 0;


    // ID sequence number (16 bits): unused, since single datagram
    iphdr.ip_id = htons (0);

    // Flags, and Fragmentation offset (3, 13 bits): 0 since single datagram

    // Zero (1 bit)
    ip_flags[0] = 0;

    // Do not fragment flag (1 bit)
    ip_flags[1] = 0;

    // More fragments following flag (1 bit)
    ip_flags[2] = 0;

    // Fragmentation offset (13 bits)
    ip_flags[3] = 0;

    iphdr.ip_off = htons ((ip_flags[0] << 15)
            + (ip_flags[1] << 14)
            + (ip_flags[2] << 13)
            +  ip_flags[3]);

    // Time-to-Live (8 bits): default to maximum value
    iphdr.ip_ttl = 255;

    // Transport layer protocol (8 bits): 17 for UDP
    iphdr.ip_p = IPPROTO_UDP;

    iphdr.ip_src = src_ip;

    // Source port number (16 bits): pick a number
    udphdr.source = htons (src_port);

    if ((socket_descriptor = socket (PF_PACKET, SOCK_DGRAM, htons (ETH_P_ALL))) < 0) {
		perror ("socket() failed ");
		return 3;
	}

	return 0;
}

int SendThroughInterface::send(char *a_data, int a_datalen, struct in_addr dst_ip, int dst_port){
	// Total length of datagram (16 bits): IP header + UDP header + datalen
//	int bytes = 0;
	iphdr.ip_len = htons (IP4_HDRLEN + UDP_HDRLEN + a_datalen);
	iphdr.ip_dst = dst_ip;
	iphdr.ip_sum = 0;
	iphdr.ip_sum = checksum ((uint16_t *) &iphdr, IP4_HDRLEN);

	// Destination port number (16 bits): pick a number
    udphdr.dest = htons (dst_port);

    // Length of UDP datagram (16 bits): UDP header + UDP data
    udphdr.len = htons (UDP_HDRLEN + a_datalen);

    // UDP checksum (16 bits)
    udphdr.check = udp4_checksum (iphdr, udphdr, (uint8_t *)a_data, a_datalen);

    // Fill out ethernet frame header.

    // Ethernet frame length = ethernet data (IP header + UDP header + UDP data)
    int frame_length = IP4_HDRLEN + UDP_HDRLEN + a_datalen;

    // IPv4 header
    memcpy (ether_frame, &iphdr, IP4_HDRLEN);

    // UDP header
    memcpy (ether_frame + IP4_HDRLEN, &udphdr, UDP_HDRLEN);

    // UDP data
    memcpy (ether_frame + IP4_HDRLEN + UDP_HDRLEN, a_data, a_datalen);

    // Send ethernet frame to socket.
    return sendto (socket_descriptor, ether_frame, frame_length, 0, (struct sockaddr *) &device, sizeof (device));
}

// Computing the internet checksum (RFC 1071).
// Note that the internet checksum does not preclude collisions.
uint16_t SendThroughInterface::checksum (uint16_t *addr, int len)
{
    int count = len;
    register uint32_t sum = 0;
    uint16_t answer = 0;

    // Sum up 2-byte values until none or only one byte left.
    while (count > 1) {
        sum += *(addr++);
        count -= 2;
    }

    // Add left-over byte, if any.
    if (count > 0) {
        sum += *(uint8_t *) addr;
    }

    // Fold 32-bit sum into 16 bits; we lose information by doing this,
    // increasing the chances of a collision.
    // sum = (lower 16 bits) + (upper 16 bits shifted right 16 bits)
    while (sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }

    // Checksum is one's compliment of sum.
    answer = ~sum;

    return (answer);
}

// Build IPv4 UDP pseudo-header and call checksum function.
uint16_t SendThroughInterface::udp4_checksum (struct ip iphdr, struct udphdr udphdr, uint8_t *payload, int payloadlen)
{
    char buf[IP_MAXPACKET];
    char *ptr;
    int chksumlen = 0;
    int i;

    ptr = &buf[0];  // ptr points to beginning of buffer buf

    // Copy source IP address into buf (32 bits)
    memcpy (ptr, &iphdr.ip_src.s_addr, sizeof (iphdr.ip_src.s_addr));
    ptr += sizeof (iphdr.ip_src.s_addr);
    chksumlen += sizeof (iphdr.ip_src.s_addr);

    // Copy destination IP address into buf (32 bits)
    memcpy (ptr, &iphdr.ip_dst.s_addr, sizeof (iphdr.ip_dst.s_addr));
    ptr += sizeof (iphdr.ip_dst.s_addr);
    chksumlen += sizeof (iphdr.ip_dst.s_addr);

    // Copy zero field to buf (8 bits)
    *ptr = 0; ptr++;
    chksumlen += 1;

    // Copy transport layer protocol to buf (8 bits)
    memcpy (ptr, &iphdr.ip_p, sizeof (iphdr.ip_p));
    ptr += sizeof (iphdr.ip_p);
    chksumlen += sizeof (iphdr.ip_p);

    // Copy UDP length to buf (16 bits)
    memcpy (ptr, &udphdr.len, sizeof (udphdr.len));
    ptr += sizeof (udphdr.len);
    chksumlen += sizeof (udphdr.len);

    // Copy UDP source port to buf (16 bits)
    memcpy (ptr, &udphdr.source, sizeof (udphdr.source));
    ptr += sizeof (udphdr.source);
    chksumlen += sizeof (udphdr.source);

    // Copy UDP destination port to buf (16 bits)
    memcpy (ptr, &udphdr.dest, sizeof (udphdr.dest));
    ptr += sizeof (udphdr.dest);
    chksumlen += sizeof (udphdr.dest);

    // Copy UDP length again to buf (16 bits)
    memcpy (ptr, &udphdr.len, sizeof (udphdr.len));
    ptr += sizeof (udphdr.len);
    chksumlen += sizeof (udphdr.len);

    // Copy UDP checksum to buf (16 bits)
    // Zero, since we don't know it yet
    *ptr = 0; ptr++;
    *ptr = 0; ptr++;
    chksumlen += 2;

    // Copy payload to buf
    memcpy (ptr, payload, payloadlen);
    ptr += payloadlen;
    chksumlen += payloadlen;

    // Pad to the next 16-bit boundary
    for (i=0; i<payloadlen%2; i++, ptr++) {
        *ptr = 0;
        ptr++;
        chksumlen++;
    }

    return checksum ((uint16_t *) buf, chksumlen);
}
