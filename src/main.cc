/*
 * main.c
 *
 *  Created on: 26-Nov-2016
 *      Author: abhijit
 */
#include "searchmac.h"
#include "SendThroughInterface.h"
#include <iostream>


int main(){
	std::map<char *, in_addr_t, cmp_str> intf2gw;
	std::map<char *, SendThroughInterface*> intf2send;
	std::map<char *, in_addr_t, cmp_str> intf2ip;
	struct ether_addr e_addr;
	char *ip;
	SendThroughInterface *snd;
	intf2gw = getAllGatewayAndIface();
	intf2ip = getAllIp();
	for (std::map<char*, in_addr_t>::iterator it = intf2gw.begin(); it != intf2gw.end(); ++it) { // calls a_map.begin() and a_map.end()
		ip = inet_ntoa(*(struct in_addr *) &(it->second));
		if(getMacR(ip, NULL, &e_addr) == 0)
		std::cout << it->first << ", " << ip << " " << ether_ntoa(&e_addr);
		if(intf2ip.find(it->first) != intf2ip.end()){
			std::cout << " " << inet_ntoa(*(struct in_addr *) &(intf2ip[it->first]));
			snd = new SendThroughInterface(it->first, e_addr, *(struct in_addr *) &(intf2ip[it->first]), 9898);
			intf2send[it->first] = snd;
			snd->init();
			in_addr_t dst_ip = inet_addr("10.5.28.210");
			for(int i = 0; i < 3; i++)
			snd->send((char *)"adata", 5, *(struct in_addr *) (&dst_ip), 6789);
		}
		std::cout << std::endl;
	}
	return 0;
}
