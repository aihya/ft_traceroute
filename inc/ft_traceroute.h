#ifndef FT_TRACEROUTE
# define FT_TRACEROUTE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include "libft.h"

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

typedef struct  s_socket
{
    int     fd;
    fd_set  read_set;
    fd_set  send_set;
    fd_set  expt_set;
}   t_socket;

typedef struct  s_packet
{
    char    buff[sizeof(struct icmphdr) + sizeof(struct timeval) + 40];
}   t_packet;

typedef struct  s_traceroute
{
    t_destinfo  dinfo;
    t_options   options;
    t_socket    socket;
    t_packet    packet;
    char        *target;

}   t_traceroute;

extern t_traceroute g_data;

#endif