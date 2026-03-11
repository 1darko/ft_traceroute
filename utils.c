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