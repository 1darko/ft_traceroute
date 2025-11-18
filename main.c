#include "traceroute.h"


/*
Bonuses:
         -N num_probes
    
         Set  the  number of probes to be sent simultaneously. The default is 3.

         -z waittime
            Wait waittime seconds between probes (default 0). This option can be used to

            slow down the sending rate of probes.

        -w waittime
            Set the time (in seconds) to wait for a response to a probe.  The default is 3 seconds.

        

To  speed  up work, normally several probes are sent simultaneously.  On the other hand, it cre‐
       ates a "storm of packages", especially in the reply direction. Routers can throttle the rate  of
       icmp  responses, and some of replies can be lost. To avoid this, decrease the number of simulta‐
       neous probes, or even set it to 1 (like in initial traceroute implementation), i.e.  -N 1

       The final (target) host can drop some of the simultaneous probes, and might even answer only the
       latest ones. It can lead to extra "looks like expired" hops near the final hop. We use  a  smart
       algorithm  to  auto-detect  such  a situation, but if it cannot help in your case, just use -N 1
       too.

       For even greater stability you can slow down the program's work by -z option, for example use -z
       0.5 for half-second pause between probes.

       To avoid an extra waiting, we use adaptive algorithm for timeouts (see -w option for more info).
       It can lead to premature expiry (especially when response times differ at  times)  and  printing
       "*"  instead of a time. In such a case, switch this algorithm off, by specifying -w with the de‐
       sired timeout only (for example, -w 5).

       If some hops report nothing for every method, the last chance to obtain something is to use ping
       -R command (IPv4, and for nearest 8 hops only).

*/

int resolve_ip(const char *name_or_ip, struct sockaddr_in *dest_addr);
char* name_resolve(struct sockaddr_in recv_addr);

int main(int ac, char **av)
{
    if(ac != 2){
        fprintf(stderr, "Usage: sudo ft_traceroute <destination>\n");
        return 1;
    }

    struct sockaddr_in dest_addr = {.sin_family = AF_INET, .sin_port = htons(STARTING_PORT)};
    if(resolve_ip(av[1], &dest_addr) != 0)
    {
    //     printf("Traceroute to %s (%s), %d hops max\n",
    //         av[1],
    //         inet_ntoa(dest_addr.sin_addr), MAX_HOPS);
    // }
    // else{
        fprintf(stderr, "ft_traceroute: unknown host %s\n", av[1]);
        return 1;
    };
    traceroute_options options = {0};
    // options_filler(&options, av);

    int send_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(send_sock < 0){
        fprintf(stderr, "error creating socket\n");
        return 1;
    };
    int recv_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(recv_sock < 0){
        fprintf(stderr, "error creating raw socket\n");
        return 1;
    };
    struct timeval tv;
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;
    setsockopt(recv_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    printf("Traceroute to %s (%s), %d hops max, %d probes per hop\n",
        av[1],
        inet_ntoa(dest_addr.sin_addr), MAX_HOPS, PROBES_TO_SEND);


    char buf[1500];
    for(int ttl = 1; ttl <= MAX_HOPS; ttl++){
        for(int probe = 0; probe < PROBES_TO_SEND; probe++){
            memset(buf, 0, sizeof(buf));
            if(setsockopt(send_sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0){
                fprintf(stderr, "error setting socket option TTL\n");
                return 1;
            };
            unsigned short port = STARTING_PORT + ttl;
            dest_addr.sin_port = htons(port);
            char msg[] = "Trace the road plz";
            struct timeval start, end;
            gettimeofday(&start, NULL);
            if(sendto(send_sock, msg, sizeof(msg), 0, (struct sockaddr *)&dest_addr,\
                sizeof(dest_addr)) < 0){
                fprintf(stderr, "error sending probe #%d\n", ttl);
                return 1;
            };
            struct sockaddr_in recv_addr;
            socklen_t addr_len = sizeof(recv_addr);
            ssize_t n = recvfrom(recv_sock, buf, sizeof(buf), 0, (struct sockaddr *)&recv_addr, &addr_len);
            gettimeofday(&end, NULL);
            double rtt = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
            if(probe == 0){
                char *resolved_name = name_resolve(recv_addr);
                printf("%2d  %s (%s)  ", resolved_name, ttl, inet_ntoa(recv_addr.sin_addr));
                free(resolved_name);
            }
            n < 0 ? ((probe + 1 == PROBES_TO_SEND)  ? printf("*\n") : printf("*  ")) : ((probe + 1 == PROBES_TO_SEND) ? printf("%.3f ms\n", rtt) : printf("%.3f ms  ", rtt));
        };
        struct iphdr *ip = (struct iphdr *)buf;
        int iphdr_len = ip->ihl * 4;
        struct icmphdr *icmp = (struct icmphdr *)(buf + iphdr_len);
        if(icmp->type == ICMP_DEST_UNREACH && icmp->code == ICMP_PORT_UNREACH){
            break;
        };
    };
    close(send_sock);
    close(recv_sock);
    return 0;
}



































char* name_resolve(struct sockaddr_in recv_addr){
    char host[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &recv_addr.sin_addr, host, sizeof(host));
    return strdup(host);
}

int resolve_ip(const char *name_or_ip, struct sockaddr_in *dest_addr)
{
    struct addrinfo hints;
    struct addrinfo *res = NULL;
    int rc;
    // char ipstr[INET_ADDRSTRLEN];
    // if (!name_or_ip || !(*resolved_ip))
        // return 1;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;        /* force IPv4 */
    hints.ai_socktype = 0;            /* pas de contrainte */

    rc = getaddrinfo(name_or_ip, NULL, &hints, &res);
    if(rc == 0)
    {
        dest_addr->sin_addr = ((struct sockaddr_in *)res->ai_addr)->sin_addr;
        // *resolved_ip = sin->sin_addr.s_addr; /* déjà en network byte order */
        // char ipstr[INET_ADDRSTRLEN];
        // inet_ntop(AF_INET, &sin->sin_addr, ipstr, sizeof(ipstr));
        // printf("%s -> %s\n", name_or_ip, ipstr);
        // dest_addr->sin_addr = sin->sin_addr;
        freeaddrinfo(res);
        return 0;
    }
    return 1;
}