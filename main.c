#include "traceroute.h"


/*
    TODO :
        Impelment packet size option
        Port option
        

        

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
int ft_strcmp(const char *s1, const char *s2);
int isnumeric(const char *str);
void print_help(void);
void print_invalid_option(const char *opt);
size_t ft_strlen(const char *s);
void	*ft_memset(void *s, int c, size_t n);
int port_range(char *a);




int init_sockets(int *send_sock, int *recv_sock)
{
    *send_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (*send_sock < 0)
        return (fprintf(stderr, "error creating socket\n"), 1);

    *recv_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (*recv_sock < 0)
        return (fprintf(stderr, "error creating raw socket\n"), 1);

    return 0;
}

struct timeval init_timeout(int timeout_value)
{
    struct timeval tv;
    tv.tv_sec  = timeout_value;
    tv.tv_usec = (timeout_value == 0) ? 1 : 0;
    return tv;
}

int send_probe(int send_sock, struct sockaddr_in *dest_addr, char *msg, size_t msg_len,
               int ttl, traceroute_options *opts, int probe)
{
    if (setsockopt(send_sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0)
        return (fprintf(stderr, "%s\n", strerror(errno)), 1);

    unsigned short port = opts->starting_port_value + (opts->starting_port_set ? ttl - 1 : 0);
    dest_addr->sin_port = htons(port);

    if (sendto(send_sock, msg, msg_len, 0,
               (struct sockaddr *)dest_addr, sizeof(*dest_addr)) < 0)
        return (fprintf(stderr, "%s\n", strerror(errno)), 1);

    (void)probe;
    return 0;
}

ssize_t receive_probe(int recv_sock, char *buf, size_t buf_len,
                      struct sockaddr_in *recv_addr, struct timeval *tv,
                      traceroute_options *opts, int *probes_sent)
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(recv_sock, &readfds);

    int activity = select(recv_sock + 1, &readfds, NULL, NULL, tv);
    if (activity) {
        socklen_t addr_len = sizeof(*recv_addr);
        ssize_t n = recvfrom(recv_sock, buf, buf_len, 0,
                             (struct sockaddr *)recv_addr, &addr_len);
        if (n < 0)
            fprintf(stderr, "%s\n", strerror(errno));
        return n;
    }
    (*probes_sent)++;
    if (*probes_sent % 16 == 0)
        *tv = init_timeout(opts->timeout_value);
    return -1;
}

void print_probe_result(int probe, ssize_t n, int ttl,
                        struct sockaddr_in recv_addr, double rtt,
                        traceroute_options *opts)
{
    if (probe == 0 && n >= 0) {
        char *resolved_name = name_resolve(recv_addr);
        printf("%2d  %s (%s)  ", ttl,
               resolved_name ? resolved_name : inet_ntoa(recv_addr.sin_addr),
               inet_ntoa(recv_addr.sin_addr));
        if(resolved_name){
            free(resolved_name);
            resolved_name = NULL;
        }               
    }
    else if (probe == 0 && n < 0)
        printf("%2d  ", ttl);

    int last_probe = (probe + 1 == opts->q_probes_value);
    if (n < 0)
        printf(last_probe ? "*" : "*  ");
    else
        printf(last_probe ? "%.3f ms" : "%.3f ms  ", rtt);
}

int traceroute_loop(int send_sock, int recv_sock, struct sockaddr_in *dest_addr,
                    traceroute_options *opts)
{
    char           msg[32];
    char           buf[1500];
    struct timeval tv = init_timeout(opts->timeout_value);
    struct timeval start, end;
    struct sockaddr_in recv_addr;
    int            probes_sent = 0;

    ft_memset(msg, 'A', sizeof(msg));
    for (int ttl = opts->first_ttl_value; ttl <= opts->max_hops_value; ttl++) {
        int last_hop = (ttl == opts->max_hops_value);
        ssize_t n = 0;

        for (int probe = 0; probe < opts->q_probes_value; probe++) {
            ft_memset(buf, 0, sizeof(buf));
            gettimeofday(&start, NULL);
            if (send_probe(send_sock, dest_addr, msg, sizeof(msg), ttl, opts, probe))
                return 1;
            n = receive_probe(recv_sock, buf, sizeof(buf), &recv_addr, &tv, opts, &probes_sent);
            gettimeofday(&end, NULL);

            double rtt = (end.tv_sec  - start.tv_sec)  * 1000.0
                       + (end.tv_usec - start.tv_usec) / 1000.0;

            print_probe_result(probe, n, ttl, recv_addr, rtt, opts);

            if ((opts->wait_between_probes_value && (probe == 0 || probe + 1 != opts->q_probes_value)) || !last_hop) {
                fflush(stdout);
                sleep(opts->wait_between_probes_value);
            }
        }
        printf("\n");
        if (n < 0)
            continue;
        struct iphdr   *ip   = (struct iphdr *)buf;
        struct icmphdr *icmp = (struct icmphdr *)(buf + ip->ihl * 4);
        if (icmp->type == ICMP_DEST_UNREACH && icmp->code == ICMP_PORT_UNREACH)
            break;
    }
    return 0;
}

int main(int ac, char **av)
{
    if (ac < 2)
        return (print_help(), 1);
    if (getuid() != 0)
        return (fprintf(stderr, "ft_traceroute: must be run as root\n"), 1);

    char               *dst_ip  = NULL;
    traceroute_options  options = {0};
    if (parser(ac, av, &options, &dst_ip))
        return 1;

    struct sockaddr_in dest_addr = {
        .sin_family = AF_INET,
        .sin_port   = htons(options.starting_port_value)
    };
    if (resolve_ip(dst_ip, &dest_addr) != 0)
        return (fprintf(stderr, "ft_traceroute: unknown host %s\n", dst_ip), 1);

    int send_sock, recv_sock;
    if (init_sockets(&send_sock, &recv_sock))
        return 1;

    char msg[32];
    printf("Setting timeout to %d seconds\n", options.timeout_value);
    printf("Traceroute to %s (%s), %d hops max, %ld bytes packet\n",
        dst_ip, inet_ntoa(dest_addr.sin_addr), options.max_hops_value,
        sizeof(msg) + sizeof(struct iphdr) + sizeof(struct udphdr));

    int ret = traceroute_loop(send_sock, recv_sock, &dest_addr, &options);
    close(send_sock);
    close(recv_sock);
    return ret;
}


// int main(int ac, char **av)
// {
//     if(ac < 2){
//         print_help();
//         return 1;
//     }
//     if(getuid() != 0){
//         fprintf(stderr, "ft_traceroute: must be run as root\n");
//         return 1;
//     };

//     char *dst_ip = NULL;
//     traceroute_options options = {0};
//     if(parser(ac, av, &options, &dst_ip))
//         return 1;
//     struct sockaddr_in dest_addr = {.sin_family = AF_INET, .sin_port = htons(options.starting_port_value)};
//     if(resolve_ip(dst_ip, &dest_addr) != 0){
//         fprintf(stderr, "ft_traceroute: unknown host %s\n", dst_ip);
//         return 1;
//     };
//     int send_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//     if(send_sock < 0){
//         fprintf(stderr, "error creating socket\n");
//         return 1;
//     };
//     int recv_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
//     if(recv_sock < 0){
//         fprintf(stderr, "error creating raw socket\n");
//         return 1;
//     };
//     struct timeval tv;
//     printf("Setting timeout to %d seconds\n", options.timeout_value);
//     tv.tv_sec = options.timeout_value;
//     if(options.timeout_value == 0){
//         tv.tv_usec = 1;
//     }
//     else{
//         tv.tv_usec = 0;
//     }
//     // setsockopt(recv_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

//     char msg[32];
//     ft_memset(msg, 'A', sizeof(msg));
//     printf("Traceroute to %s (%s), %d hops max, %ld bytes packet\n",
//         dst_ip,inet_ntoa(dest_addr.sin_addr), options.max_hops_value, sizeof(msg) + sizeof(struct iphdr) + sizeof(struct udphdr));


//     char buf[1500];
//     ssize_t n = 0;
//     struct timeval start, end;
//     struct sockaddr_in recv_addr;
//     int last_hop = 0;
//     int probes_sent = 0;
//     for(int ttl = options.first_ttl_value; ttl <= options.max_hops_value; ttl++){
//         if(ttl == options.max_hops_value)
//             last_hop = 1;
//         for(int probe = 0; probe < options.q_probes_value; probe++){
//             ft_memset(buf, 0, sizeof(buf));
//             if(setsockopt(send_sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0){
//                 fprintf(stderr, "%s\n", strerror(errno));
//                 return 1;
//             };
//             unsigned short port = options.starting_port_value + (options.starting_port_set ? ttl - 1 : 0);
//             dest_addr.sin_port = htons(port);
//             gettimeofday(&start, NULL);
//             if(sendto(send_sock, msg, sizeof(msg), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0){
//                 fprintf(stderr, "%s\n", strerror(errno));
//                 return 1;
//             };
//             //Solution #2 : select before recvfrom
//             fd_set readfds;
//             FD_ZERO(&readfds);
//             FD_SET(recv_sock, &readfds);
//             int activity = select(recv_sock + 1, &readfds, NULL, NULL, &tv);
//             if(activity){
//                 socklen_t addr_len = sizeof(recv_addr);
//                 n = recvfrom(recv_sock, buf, sizeof(buf), 0, (struct sockaddr *)&recv_addr, &addr_len);
//                 // A voir la gestion error
//                 if(n < 0){
//                     fprintf(stderr, "%s\n", strerror(errno));
//                     return 1;
//                 }
//             }
//             else{                
//                 probes_sent++;
//                 if(probes_sent % 16 == 0){
//                     tv.tv_sec = options.timeout_value;
//                     if(options.timeout_value == 0){
//                         tv.tv_usec = 1;
//                     }
//                     else{
//                         tv.tv_usec = 0;
//                     }
//                 }
//                 n = -1;
//             }
//             gettimeofday(&end, NULL);
//             // Solution #1: reset timeout after each recvfrom
//             // if(n < 0){
//             //     tv.tv_sec = 1;
//             //     tv.tv_usec = 0;
//             //     setsockopt(recv_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
//             // }
//             double rtt = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
//             if(probe == 0 && n >= 0){
//                 char *resolved_name = name_resolve(recv_addr);
//                 printf("%2d  %s (%s)  ", ttl, (resolved_name != NULL ? resolved_name : inet_ntoa(recv_addr.sin_addr)), inet_ntoa(recv_addr.sin_addr));
//                 free(resolved_name);
//             }
//             else if(probe == 0 && n < 0){
//                 printf("%2d  ", ttl);
//             }
//             n < 0 ? ((probe + 1 == options.q_probes_value)  ? printf("*") : printf("*  ")) : ((probe + 1 == options.q_probes_value) ? printf("%.3f ms", rtt) : printf("%.3f ms  ", rtt));
//             if((options.wait_between_probes_value && (probe == 0 || probe + 1 != options.q_probes_value)) || !last_hop){
//                 fflush(stdout);
//                 sleep(options.wait_between_probes_value);
//             }
//         };
//         printf("\n");
//         if(n < 0)
//             continue;
//         struct iphdr *ip = (struct iphdr *)buf;
//         int iphdr_len = ip->ihl * 4;
//         struct icmphdr *icmp = (struct icmphdr *)(buf + iphdr_len);
//         if(icmp->type == ICMP_DEST_UNREACH && icmp->code == ICMP_PORT_UNREACH){
//             break;
//         };
//     };
//     close(send_sock);
//     close(recv_sock);
//     return 0;
// }




// char* name_resolve(struct sockaddr_in recv_addr){
//     char host[INET_ADDRSTRLEN];
//     if(inet_ntop(AF_INET, &recv_addr.sin_addr, host, sizeof(host)) == NULL)
//         return NULL;
//     if(getnameinfo((struct sockaddr *)&recv_addr, sizeof(recv_addr), host, sizeof(host), NULL, 0, 0) != 0)
//         return NULL;
//     return strdup(host);
// }

