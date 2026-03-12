#include "traceroute.h"


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
static char	*ft_strdup(const char *s)
{
	size_t	len;
	size_t	cur;
	char	*dup;

	cur = 0;
	len = ft_strlen((char *)s);
	dup = (char *)malloc(sizeof(char) * len + 1);
	if (dup == 0)
		return (NULL);
	while (s[cur] != '\0')
	{
		dup[cur] = s[cur];
	cur++;
	}
	dup[cur] = '\0';
	return (dup);
}

char* name_resolve(struct sockaddr_in recv_addr) {
    char host[NI_MAXHOST]; 
    if (getnameinfo((struct sockaddr *)&recv_addr, sizeof(recv_addr),
                    host, sizeof(host), NULL, 0, 0) != 0) {
        return NULL;
    }
    return ft_strdup(host);
}

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

