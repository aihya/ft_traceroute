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
# include "libft.h"

# define USE_ICMP    (g_data.options.I)
# define PACKET_SIZE (sizeof(struct icmphdr))
# define SOCK_FD     g_data.socket.fd
# define READ_FDS    g_data.socket.readfds
# define PRESENT_SIZE 256

typedef struct  s_destinfo
{
    struct addrinfo *result;
    struct addrinfo ai;
}   t_destinfo;

typedef struct  s_options
{
    int         opt;
    long long   m;  // max_ttl
    long long   M;  // first_ttl
    long long   q;  // Max number probs per ttl/hop
    int         n;  // Print numerical address only
    int         w;  // waittime
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
    fd_set  fds;
}   t_socket;

typedef struct s_presentable
{
    char    target[PRESENT_SIZE];
    char    current[PRESENT_SIZE];
}   t_presentable;

typedef struct  s_traceroute
{
    t_destinfo      dinfo;
    t_options       options;
    t_socket        socket;
    t_presentable   presentable;
    size_t          sequence;
    char            packet[PACKET_SIZE];
    char            *target;
    struct timeval  send_time;
    struct timeval  recv_time;
    int             protocol;
    int             type;
    t_sockaddrs     sa;
}   t_traceroute;

extern t_traceroute g_data;

#endif