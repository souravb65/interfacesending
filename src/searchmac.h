/*
 * searchmac.h
 *
 *  Created on: 26-Nov-2016
 *      Author: abhijit
 */

#ifndef SEARCHMAC_H_
#define SEARCHMAC_H_


#include <stdio.h>
#include <string.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <map>
#include <ifaddrs.h>

#define BUFFER_SIZE 4096
#define CONT_LEN 40

struct cmp_str
{
   bool operator()(char const *a, char const *b)
   {
      return strcmp(a, b) < 0;
   }
};

std::map<char *, in_addr_t, cmp_str> getAllGatewayAndIface();
std::map<char *, in_addr_t, cmp_str> getAllIp();
int getGatewayAndIface(in_addr_t * addr, char *interface);
int getMac(char *searchIp, char *chwaddr, struct ether_addr *hwaddr);
int getMacR(char *searchIp, char *c_hwaddr, struct ether_addr *e_hwaddr);
int getGatewayAddress(char *iface, in_addr_t *a_ip, char *s_ip, struct ether_addr *e_hwaddr, char *c_hwaddr);
#endif /* SEARCHMAC_H_ */
