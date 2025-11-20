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


void	*ft_memset(void *s, int c, size_t n)
{
	size_t			cur;
	unsigned char	*temp;

	temp = (unsigned char *) s;
	cur = 0;
	while (cur < n)
	{
		*(temp++) = (unsigned char)c;
		cur++;
	}
	return (s);
};


size_t	ft_strlen(const char *str)
{
	size_t	i;

	i = 0;
	while (str[i])
		i++;
	return (i);
};

int int_overflow(char *a)
{
    if(!a)
        return 0;
    if(ft_strlen(a) > 10)
        return 1;
    if(ft_strlen(a) == 10){
        if(ft_strcmp(a, "2147483647") > 0)
            return 1;
    }
    return 0;
};

int port_range(char *a)
{
    if(!a)
        return 0;
    if(ft_strlen(a) > 5 || a[0] == '-' || a[0] == '0')
        return 1;
    if(ft_strlen(a) == 5){
        if(ft_strcmp(a, "65535") > 0)
            return 1;
    }
    return 0;
};
void value_error(const char *value, char near_char, int type){
    fprintf(stderr, "ft_traceroute: invalid value (`%s' near `%c')\n", type ? value + 2 : value, near_char);
    fprintf(stderr, "Try 'ft_traceroute --help' for more information.\n");
};


int value_check(traceroute_options *opts)
{
    // if(opts->q_probes <= 0 || opts->s_value > 1473){
    //     fprintf(stderr, "ft_traceroute: invalid payload size '%d'\n", opts->s_value);
    //     return 1;
    // }
    if(opts->q_probes && (atoi(opts->q_probes) <= 0 || int_overflow(opts->q_probes))){
        fprintf(stderr, "ft_traceroute: invalid probes '%s'\n", opts->q_probes);
        return 1;
    }
    if(opts->wait_between_probes && (atoi(opts->wait_between_probes) <= 0 || int_overflow(opts->wait_between_probes))){
        fprintf(stderr, "ft_traceroute: invalid wait time '%s'\n", opts->wait_between_probes);
        return 1;
    }
    if(opts->timeout && (atoi(opts->timeout) < 0 || int_overflow(opts->timeout))){
        fprintf(stderr, "ft_traceroute: invalid timeout '%s'\n", opts->timeout);
        return 1;
    }
    if(opts->first_ttl && (atoi(opts->first_ttl) < 1 || int_overflow(opts->first_ttl))){
        fprintf(stderr, "ft_traceroute: invalid first TTL '%s'\n", opts->first_ttl);
        return 1;
    }
    // if(opts->starting_port && port_range(opts->starting_port)){
    //     fprintf(stderr, "ft_traceroute: invalid starting port '%s'\n", opts->starting_port);
    //     return 1;
    // }
    if(opts->max_hops && (atoi(opts->max_hops) < 1 || int_overflow(opts->max_hops))){
        fprintf(stderr, "ft_traceroute: invalid max hops '%s'\n", opts->max_hops);
        return 1;
    }
    return 0;
}

int parser(int ac, char **av, traceroute_options *opts, char **dst_ip)
{
    int break_flag;
    int ip_found = 0;
    for(int i = 1; i < ac; i++){
        break_flag = 0;
        if(av[i][0] == '-'){
            for(int j = 1; av[i][j] != '\0' && !break_flag; j++){
                switch(av[i][j]){
                    case '-':
                        ft_strcmp(av[i], "--help") == 0 ? print_help() : print_invalid_option(av[i]);
                        return 1;
                    case 'q':
                        if(av[i][j + 1] != '\0'){
                            if(!isnumeric(&av[i][j + 1])){
                                return (value_error(av[i], 'q', 1), 1);
                            }
                            opts->q_probes = &av[i][j + 1];
                            break_flag = 1;
                            break;
                        }
                        else if(++i < ac && av[i][0] != '\0') {
                            if(!isnumeric(av[i])){
                                return (value_error(av[i], 'q', 0), 1);
                            }
                            opts->q_probes = av[i];
                        }
                        else{
                            fprintf(stderr, "ft_traceroute: option '-q' requires an argument\n");
                            return 1;
                        }
                        break_flag = 1;
                        break;
                    case 'z':
                        if(av[i][j + 1] != '\0'){
                            if(!isnumeric(&av[i][j + 1])){
                                return (value_error(av[i], 'z', 1), 1);
                            }
                            opts->wait_between_probes = &av[i][j + 1];
                        }
                        else if(++i < ac && av[i][0] != '\0') {
                            if(!isnumeric(av[i])){
                                return (value_error(av[i], 'z', 0), 1);
                            }
                            opts->wait_between_probes = av[i];
                        }
                        else{
                            fprintf(stderr, "ft_traceroute: option '-z' requires an argument\n");
                            return 1;
                        }
                        break_flag = 1;
                        break;
                    case 'w':
                        if(av[i][j + 1] != '\0'){
                            if(!isnumeric(&av[i][j + 1])){
                                return (value_error(av[i], 'w', 1), 1);
                            }
                            opts->timeout = &av[i][j + 1];
                        }
                        else if(++i < ac && av[i][0] != '\0') {
                            if(!isnumeric(av[i])){
                                return (value_error(av[i], 'w', 0), 1);
                            }
                            opts->timeout = av[i];
                        }
                        else{
                            fprintf(stderr, "ft_traceroute: option '-w' requires an argument\n");
                            return 1;
                        }
                        break_flag = 1;
                        break;
                    case 'm':
                        if(av[i][j + 1] != '\0'){
                            if(!isnumeric(&av[i][j + 1])){
                                return (value_error(av[i], 'm', 1), 1);
                            }
                            opts->max_hops = &av[i][j + 1];
                        }
                        else if(++i < ac && av[i][0] != '\0') {
                            if(!isnumeric(av[i])){
                                return (value_error(av[i], 'm', 0), 1);
                            }
                            opts->max_hops = av[i];
                        }
                        else{
                            fprintf(stderr, "ft_traceroute: option '-m' requires an argument\n");
                            return 1;
                        }
                        break_flag = 1;
                        break;
                    case 'p':
                        if(av[i][j + 1] != '\0'){
                            if(!isnumeric(&av[i][j + 1])){
                                return (value_error(av[i], 'p', 1), 1);
                            }
                            opts->starting_port = &av[i][j + 1];
                        }
                        else if(++i < ac && av[i][0] != '\0') {
                            if(!isnumeric(av[i])){
                                return (value_error(av[i], 'p', 0), 1);
                            }
                            opts->starting_port = av[i];
                        }
                        else{
                            fprintf(stderr, "ft_traceroute: option '-p' requires an argument\n");
                            return 1;
                        }
                        break_flag = 1;
                        break;
                    case 'f':
                        if(av[i][j + 1] != '\0'){
                            if(!isnumeric(&av[i][j + 1])){
                                return (value_error(av[i], 'f', 1), 1);
                            }
                            opts->first_ttl = &av[i][j + 1];
                        }
                        else if(++i < ac && av[i][0] != '\0') {
                            if(!isnumeric(av[i])){
                                return (value_error(av[i], 'f', 0), 1);
                            }
                            opts->first_ttl = av[i];
                        }
                        else{
                            fprintf(stderr, "ft_traceroute: option '-f' requires an argument\n");
                            return 1;
                        }
                        break_flag = 1;
                        break;
                    default:
                        fprintf(stderr, "ft_traceroute: invalid option -- '%c'\n", av[i][j]);
                        fprintf(stderr, "Try 'ft_traceroute --help' for more information.\n");
                        return 1;
                    }
                }
            }
            else if(ip_found == 0){
                *dst_ip = av[i];
                ip_found = 1;
            }
            else{
                fprintf(stderr, "ft_traceroute: extra argument '%s'\n", av[i]);
                fprintf(stderr, "Try 'ft_traceroute --help' for more information.\n");
                return 1;
            }
        }
        if(!ip_found){
            fprintf(stderr, "ft_traceroute: destination address required\n");
            fprintf(stderr, "Try 'ft_traceroute --help' for more information.\n");
            return 1;
    }
    if(value_check(opts))
        return 1;
    !opts->q_probes ? (opts->q_probes_value = PROBES_TO_SEND) : (opts->q_probes_value = atoi(opts->q_probes));
    if(opts->wait_between_probes)
        opts->wait_between_probes_value = atoi(opts->wait_between_probes);
    !opts->timeout ? (opts->timeout_value = TIMEOUT) : (opts->timeout_value = atoi(opts->timeout));
    !opts->max_hops ? (opts->max_hops_value = MAX_HOPS) : (opts->max_hops_value = atoi(opts->max_hops));
    !opts->first_ttl ? (opts->first_ttl_value = 1) : (opts->first_ttl_value = atoi(opts->first_ttl));
    if(!opts->starting_port){
        opts->starting_port_value = STARTING_PORT;
        opts->starting_port_set = 1;
    }
    else{
        opts->starting_port_value = atoi(opts->starting_port);
        if(opts->starting_port_value == 0)
        {
            fprintf(stderr, "ft_traceroute: invalid starting port '%s'\n", opts->starting_port);
            return 1;
        }
        opts->starting_port_set = 0;
    }

    return 0;
};


int main(int ac, char **av)
{
    if(ac < 2){
        print_help();
        return 1;
    }
    if(getuid() != 0){
        fprintf(stderr, "ft_traceroute: must be run as root\n");
        return 1;
    };

    char *dst_ip = NULL;
    traceroute_options options = {0};
    if(parser(ac, av, &options, &dst_ip))
        return 1;
    struct sockaddr_in dest_addr = {.sin_family = AF_INET, .sin_port = htons(options.starting_port_value)};
    if(resolve_ip(dst_ip, &dest_addr) != 0){
        fprintf(stderr, "ft_traceroute: unknown host %s\n", dst_ip);
        return 1;
    };
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
    printf("Setting timeout to %ld seconds\n", options.timeout_value);
    tv.tv_sec = options.timeout_value;
    if(options.timeout_value == 0){
        tv.tv_usec = 1;
    }
    else{
        tv.tv_usec = 0;
    }
    // setsockopt(recv_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    char msg[32];
    ft_memset(msg, 'A', sizeof(msg));
    printf("Traceroute to %s (%s), %d hops max, %d bytes packet\n",
        dst_ip,inet_ntoa(dest_addr.sin_addr), options.max_hops_value, sizeof(msg) + sizeof(struct iphdr) + sizeof(struct udphdr));


    char buf[1500];
    ssize_t n = 0;
    struct timeval start, end;
    struct sockaddr_in recv_addr;
    int last_hop = 0;
    int probes_sent = 0;
    for(int ttl = options.first_ttl_value; ttl <= options.max_hops_value; ttl++){
        if(ttl == options.max_hops_value)
            last_hop = 1;
        for(int probe = 0; probe < options.q_probes_value; probe++){
            ft_memset(buf, 0, sizeof(buf));
            if(setsockopt(send_sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0){
                fprintf(stderr, "%s\n", strerror(errno));
                return 1;
            };
            unsigned short port = options.starting_port_value + (options.starting_port_set ? ttl - 1 : 0);
            dest_addr.sin_port = htons(port);
            gettimeofday(&start, NULL);
            if(sendto(send_sock, msg, sizeof(msg), 0, (struct sockaddr *)&dest_addr,\
            sizeof(dest_addr)) < 0){
                fprintf(stderr, "%s\n", strerror(errno));
                return 1;
            };
            //Solution #2 : select before recvfrom
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(recv_sock, &readfds);
            int activity = select(recv_sock + 1, &readfds, NULL, NULL, &tv);
            if(activity){
                socklen_t addr_len = sizeof(recv_addr);
                n = recvfrom(recv_sock, buf, sizeof(buf), 0, (struct sockaddr *)&recv_addr, &addr_len);
            }
            else{                
                probes_sent++;
                if(probes_sent % 16 == 0){
                    tv.tv_sec = options.timeout_value;
                    if(options.timeout_value == 0){
                        tv.tv_usec = 1;
                    }
                    else{
                        tv.tv_usec = 0;
                    }
                }
                n = -1;
            }
            gettimeofday(&end, NULL);
            // Solution #1: reset timeout after each recvfrom
            // if(n < 0){
            //     tv.tv_sec = 1;
            //     tv.tv_usec = 0;
            //     setsockopt(recv_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
            // }
            double rtt = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
            if(probe == 0 && n >= 0){
                char *resolved_name = name_resolve(recv_addr);
                printf("%2d  %s (%s)  ", ttl, (resolved_name != NULL ? resolved_name : inet_ntoa(recv_addr.sin_addr)), inet_ntoa(recv_addr.sin_addr));
                free(resolved_name);
            }
            else if(probe == 0 && n < 0){
                printf("%2d  ", ttl);
            }
            n < 0 ? ((probe + 1 == options.q_probes_value)  ? printf("*") : printf("*  ")) : ((probe + 1 == options.q_probes_value) ? printf("%.3f ms", rtt) : printf("%.3f ms  ", rtt));
            if((options.wait_between_probes_value && (probe == 0 || probe + 1 != options.q_probes_value)) || !last_hop){
                fflush(stdout);
                sleep(options.wait_between_probes_value);
            }
        };
        printf("\n");
        if(n < 0)
            continue;
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
    if(inet_ntop(AF_INET, &recv_addr.sin_addr, host, sizeof(host)) == NULL)
        return NULL;
    if(getnameinfo((struct sockaddr *)&recv_addr, sizeof(recv_addr), host, sizeof(host), NULL, 0, 0) != 0)
        return NULL;
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
    ft_memset(&hints, 0, sizeof(hints));
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
};

int ft_strcmp(const char *s1, const char *s2)
{
    unsigned char *s3;
    unsigned char *s4;

    s3 = (unsigned char *)s1;
    s4 = (unsigned char *)s2;
    while(*s3 == *s4 && *s3 != '\0')
    {
        ++s3;
        ++s4;
    }
    return (*s3 - *s4);
};

int isnumeric(const char *str){
    if(str == NULL || *str == '\0')
        return 0;
    for(int i = 0; str[i] != '\0'; i++){
        if(str[i] < '0' || str[i] > '9')
            return 0;
    }
    return 1;
};

void print_help(void){
    fprintf(stderr,"Usage:\ntraceroute [ -f first_ttl ] [ -m max_ttl ] [ -q squeries ] [ -p port ] [ -w MAX,HERE,NEAR ] [ -q nqueries ] [ -z sendwait ] <hostname>\n\n");
    fprintf(stderr,"Options:\n");
    fprintf(stderr,"  -f first_ttl     Set the initial time-to-live for outgoing packets. The default is 1.\n");
    fprintf(stderr,"  -m max_ttl       Set the max number of hops (max time-to-live value) traceroute will use. The default is 30.\n");
    fprintf(stderr,"  -p port          Set the base UDP port number used in probes. The default is 33434.\n");
    fprintf(stderr,"  -w time to wait  Set the timeout values for probes. The default is 5.\n");
    fprintf(stderr,"  -q nqueries      Set the number of queries to be sent. The default is 3.\n");
    fprintf(stderr,"  -z sendwait      Set the time to wait between sending probes. The default is 0.\n");
};

void print_invalid_option(const char *opt){
    fprintf(stderr, "Invalid option: %s\n", opt);
};

