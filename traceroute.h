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
#include <netinet/udp.h>

#define PROBES_TO_SEND 3
#define MAX_HOPS 30
#define WAIT_BETWEEN_PROBES 0
#define TIMEOUT 5
#define STARTING_PORT 33433

#ifndef PRINTER
# define PRINTER 0
#endif

typedef struct s_traceroute_options{
    char*     q_probes; // -q
    char*     wait_between_probes; // -z
    char*     timeout; // -w
    char*     max_hops; // -m
    char*     first_ttl; // -f
    char*     starting_port; // -p

    int      q_probes_value;
    int      wait_between_probes_value;
    int      timeout_value;
    int      max_hops_value;
    int      first_ttl_value;
    int      starting_port_value;
    int      starting_port_set;
}   traceroute_options;

/*
Bonuses:
         -q num_probes
    
         Set  the  number of probes to be sent simultaneously. The default is 3.

         -z waittime
            Wait waittime seconds between probes (default 0). This option can be used to

            slow down the sending rate of probes.

        -w waittime
            Set the time (in seconds) to wait for a response to a probe.  The default is 3 seconds.
        -f first_ttl
            Set the initial time-to-live for outgoing packets. The default is 1.
*/

void	*ft_memset(void *s, int c, size_t n);
size_t	ft_strlen(const char *str);
int int_overflow(char *a);
int port_range(char *a);
int parser(int ac, char **av, traceroute_options *opts, char **dst_ip);
int ft_strcmp(const char *s1, const char *s2);
void value_error(const char *value, char near_char, int type);
int value_check(traceroute_options *opts);
int isnumeric(const char *str);
void print_invalid_option(const char *opt);
void print_help(void);

#endif


