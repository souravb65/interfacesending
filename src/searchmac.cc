#include "searchmac.h"
std::map<char *, in_addr_t, cmp_str>  getAllGatewayAndIface()
{
    long destination, gateway;
    char iface[IF_NAMESIZE];
    char buf[BUFFER_SIZE];
    char *tmp;
    in_addr_t addr;
    std::map<char *, in_addr_t, cmp_str> interfaces;
    FILE * file;

    memset(iface, 0, sizeof(iface));
    memset(buf, 0, sizeof(buf));

    file = fopen("/proc/net/route", "r");
    if (!file)
        return interfaces;

    while (fgets(buf, sizeof(buf), file)) {
        if (sscanf(buf, "%s %lx %lx", iface, &destination, &gateway) == 3) {
            if (destination == 0) { /* default */
                addr = gateway;
                tmp = new char[strlen(iface) + 1];
                strcpy(tmp, iface);
                interfaces[tmp] = addr;
                //fclose(file);
                //return 0;
            }
        }
    }

    /* default route not found */
    if (file)
        fclose(file);
    return interfaces;
}

int getGatewayAndIface(in_addr_t * addr, char *interface)
{
    long destination, gateway;
    char iface[IF_NAMESIZE];
    char buf[BUFFER_SIZE];
    FILE * file;

    memset(iface, 0, sizeof(iface));
    memset(buf, 0, sizeof(buf));

    file = fopen("/proc/net/route", "r");
    if (!file)
        return -1;

    while (fgets(buf, sizeof(buf), file)) {
        if (sscanf(buf, "%s %lx %lx", iface, &destination, &gateway) == 3) {
            if (destination == 0 && strcmp(interface, iface) == 0) { /* default */
                *addr = gateway;
                fclose(file);
                return 0;
            }
        }
    }

    /* default route not found */
    if (file)
        fclose(file);
    return -1;
}

int getMac(char *searchIp, char *chwaddr, struct ether_addr *hwaddr){
    FILE *fp;
    char buf[BUFFER_SIZE];
    char ip[CONT_LEN], hwt[CONT_LEN], flg[CONT_LEN], hwa[CONT_LEN];
    struct ether_addr *tmaddr;

    fp = fopen("/proc/net/arp", "r");
    if(fp == NULL){
        perror("fread: ");
    }
    
    while(fgets(buf, sizeof(buf), fp)){
        if(sscanf(buf, "%s %s %s %s", ip, hwt, flg, hwa) == 4){
            if(strcmp(ip, searchIp) == 0){
                if(chwaddr != NULL){
                    strcpy(chwaddr, hwa);
                }
                if(hwaddr != NULL){
                    tmaddr = ether_aton(hwa);
                    *hwaddr = *tmaddr;
                }
                //printf("%s -> %s\n", ip, hwa);
                fclose(fp);
                return 0;
            }
        }
    }
    fclose(fp);
    return -1;
}

std::map<char *, in_addr_t, cmp_str> getAllIp(){
    struct ifaddrs *addrs, *tmp;
    int status;
    std::map<char *, in_addr_t, cmp_str> ipMap;
    char *intf;
    status = getifaddrs(&addrs);
    if (status != 0){
        return ipMap;
    }

    tmp = addrs;

    while (tmp)
    {
        if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET)
        {
            struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
            intf = new char[strlen(tmp->ifa_name) + 1];
            strcpy(intf, tmp->ifa_name);
            ipMap[intf] = pAddr->sin_addr.s_addr;
        }

        tmp = tmp->ifa_next;
    }

    freeifaddrs(addrs);
    return ipMap;
}

int getMacR(char *searchIp, char *c_hwaddr, struct ether_addr *e_hwaddr){
    char arpingcmd[100];
    if(getMac(searchIp, c_hwaddr, e_hwaddr) != 0){
        if(getMac(searchIp, c_hwaddr, e_hwaddr) != 0){
            return 2;
        }
    }
    return 0;
}

int getGatewayAddress(char *iface, in_addr_t *a_ip, char *s_ip, struct ether_addr *e_hwaddr, char *c_hwaddr){
    in_addr_t addr = 0;
    char ip[CONT_LEN], *tmp;
    if(getGatewayAndIface(&addr, iface) != 0){
        fprintf(stderr, "%s: no such device\n", iface);
        return 1;
    }
    if(a_ip){
    	*a_ip = addr;
    }
    tmp = inet_ntoa(*(struct in_addr *) &addr);
    strcpy(ip, tmp);
    if(s_ip){
    	strcpy(s_ip, tmp);
    }

    return getMacR(ip, c_hwaddr, e_hwaddr);
}

#if 0
int printgatewayaddress(char *iface){
    in_addr_t addr = 0;
    char arpingcmd[100];
    char ip[CONT_LEN], *tmp, chwaddr[CONT_LEN];
    if(getgatewayandiface(&addr, iface) != 0){
        fprintf(stderr, "%s: no such device\n", iface);
        return 1;
    }
    tmp = inet_ntoa(*(struct in_addr *) &addr);
    strcpy(ip, tmp);
    struct ether_addr hwaddr;
    if(getMac(ip, chwaddr, &hwaddr) == 0){
        printf("%s <-> %s <-> %s\n", iface, ip, chwaddr);
    }
    else{
        sprintf(arpingcmd, "arping -c 2 -I %s %s", iface, ip);
        if(getMac(ip, chwaddr, &hwaddr) == 0){
            printf("%s <-> %s <-> %s\n", iface, ip, chwaddr);
        }
        else{
            fprintf(stderr, "%s: host not reachable from this interface\n", iface);
            return 1;
        }
    }
    return 0;
}

int main(int argc, char *argv[]){
    //printf("%s <-> %s <-> %02x-%02x-%02x-%02x-%02x-%02x\n", argv[1], ip, hwaddr.ether_addr_octet[0], \
    //        hwaddr.ether_addr_octet[1], \
    //        hwaddr.ether_addr_octet[2], \
    //        hwaddr.ether_addr_octet[3], \
    //        hwaddr.ether_addr_octet[4], \
    //        hwaddr.ether_addr_octet[5]);
    return printgatewayaddress(argv[1]);
    return 0;
}
#endif
