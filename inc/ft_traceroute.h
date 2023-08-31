#ifndef FT_TRACEROUTE
# define FT_TRACEROUTE

# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>
# include <errno.h>
# include <arpa/inet.h>
# include <netinet/ip_icmp.h>
# include <netinet/ip.h>
# include <netinet/udp.h>
# include <sys/time.h>
# include <stdbool.h>
# include "libft.h"

typedef struct  s_destinfo
{
    struct addrinfo *result;
    struct addrinfo ai;
}   t_destinfo;

typedef struct  s_options
{
    int         opt;
    long long   f;
    long long   q;
    int         n;
    int         w;
    int         p;
    int         I;
}   t_options;

typedef struct s_sockaddrs
{
    struct sockaddr sa_send;
    struct sockaddr sa_recv;
    struct sockaddr sa_bind;
}   t_sockaddrs;

typedef struct  s_packet
{
    char    buff[1024];
    size_t  size;
}   t_packet;

typedef struct s_socket
{
    int     rfd;
    int     sfd;
}   t_socket;

typedef struct  s_traceroute
{
    t_destinfo      dinfo;
    t_options       options;
    t_socket        socket;
    char            *target;
    struct timeval  send_time;
    struct timeval  recv_time;
    struct in_addr  last_addr;
    int             protocol;
    int             type;
    t_sockaddrs     sa;
}   t_traceroute;

extern t_traceroute g_data;

#endif