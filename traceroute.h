#ifndef TRACEROUTE_H
# define TRACEROUTE_H

#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <net/ethernet.h>   /* ETH_ALEN */
#include <net/if.h>
#include <net/if_arp.h>     /* ARP hardware types */
#include <netpacket/packet.h> /* sockaddr_ll (AF_PACKET) */
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <limits.h>
#include <signal.h>

#define PROBES_TO_SEND 3
#define MAX_HOPS 30
#define WAIT_BETWEEN_PROBES 0
#define TIMEOUT 3
#define STARTING_PORT 33433

#ifndef PRINTER
# define PRINTER 0
#endif

typedef struct s_traceroute_options{
    int     n_probes;
    float   wait_between_probes;
    int     timeout;
    int     max_hops;
    int     starting_port;    
}   traceroute_options;



#endif


