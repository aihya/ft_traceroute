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
# include <sys/time.h>
# include "libft.h"

# define PACKET_SIZE (sizeof(struct icmphdr) + sizeof(struct timeval) + 40)
# define SOCK_FD     g_data.socket.fd
# define READ_FDS    g_data.socket.readfds
# define PRESENT_SIZE 256

typedef struct  s_destinfo
{
    struct addrinfo *result;
    struct addrinfo ai;
    struct sockaddr *sa;
    struct sockaddr_in *sin;
}   t_destinfo;

typedef struct  s_options
{
    int         opt;
    long long   m;
    long long   p;
    long long   q;
    long long   N;
}   t_options;

typedef struct s_socket
{
    int     fd;
    fd_set  readfds;
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
}   t_traceroute;

extern t_traceroute g_data;

#endif