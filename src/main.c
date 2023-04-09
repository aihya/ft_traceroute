#include "ft_traceroute.h"

t_traceroute g_data = (t_traceroute){0};

/***********************************************************/
/* Utilities                                               */
/***********************************************************/
uint16_t cksum(uint16_t *data, size_t size)
{
    uint32_t    checksum;
    size_t      count;

    checksum = 0;
    count = size;
    while (count > 1)
    {
        checksum += *data++;
        count -= 2;
    }
    if (count)
        checksum += *(uint8_t *)data;

    checksum = (checksum >> 16) + (checksum & 0xffff);
    checksum = (checksum >> 16) + checksum;

    return (~checksum);
}

void    presentable(struct in_addr addr)
{
    ft_bzero(g_data.presentable, sizeof(g_data.presentable));
    inet_ntop(AF_INET, &addr, g_data.presentable, sizeof(g_data.presentable));
}

void    resolve_destination()
{
    int             ret;
    struct addrinfo hints;
    struct addrinfo *p;

    hints = (struct addrinfo){0};
    hints.ai_family     = AF_INET;
    hints.ai_socktype   = SOCK_DGRAM;
    hints.ai_protocol   = IPPROTO_UDP;

    if ((ret = getaddrinfo(g_data.target, NULL, &hints, &g_data.dinfo.result)) < 0)
    {
        fprintf(stderr, "traceroute: %s: %s\n", g_data.target, gai_strerror(ret));
        exit(ret);
    }

    for (p = g_data.dinfo.result; p; p = p->ai_next)
    {
        if (p->ai_protocol == IPPROTO_UDP && p->ai_family == AF_INET)
            break;
    }
    if (p)
    {
        ft_memcpy(&g_data.dinfo.ai, p, sizeof(struct addrinfo));
        g_data.dinfo.sa = g_data.dinfo.ai.ai_addr;
        g_data.dinfo.sin = (struct sockaddr_in *)g_data.dinfo.sa;
        return ;
    }
    exit(0);
}


void    setup_socket()
{
    struct sockaddr_in  addr;
    
    SOCK_FD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (SOCK_FD == -1)
    {
        fprintf(stderr, "traceroute: socket: %s\n", strerror(errno));
        exit(errno);
    }

    addr = (struct sockaddr_in){0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(33434);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(SOCK_FD, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        fprintf(stderr, "traceroute: bind: %s\n", strerror(errno));
        exit(errno);
    }
}


void    send_packets()
{
    int             rv;
    struct icmphdr	*icmp;
    size_t          npackets;

	ft_memset(g_data.packet, 0x00, sizeof(g_data.packet));
	icmp = (struct icmphdr *)g_data.packet;
	icmp->type = ICMP_ECHO;
	icmp->code = 0;
	icmp->un.echo.id = (uint16_t)getpid();
	icmp->un.echo.sequence = g_data.sequence;

    npackets = 0;
    for (int npackets = 0; npackets < g_data.options.q; npackets++)
    {

        gettimeofday((void *)(icmp + 1), 0);
	    icmp->checksum = 0;
	    icmp->checksum = cksum((uint16_t *)icmp, sizeof(g_data.packet));

        rv = sendto(
            SOCK_FD, 
            g_data.packet, 
            sizeof(g_data.packet), 
            0, 
            g_data.dinfo.ai.ai_addr, 
            g_data.dinfo.ai.ai_addrlen
        );
        perror("sendto");
    }
}

void    receive_packets()
{
    int             rv;
    static char     buf[IP_MAXPACKET];
    struct sockaddr src_addr;
    socklen_t       addr_len;
    struct timeval  wait;

    wait.tv_sec = 1;
    wait.tv_usec = 0;

    FD_ZERO(&READ_FDS);
    FD_SET(SOCK_FD, &READ_FDS);

    for (int npackets = 0; npackets < g_data.options.q; npackets++)
    {

        wait.tv_sec = 5;
        wait.tv_usec = 0;
        rv = select(1, &READ_FDS, NULL, NULL, &wait);

        src_addr = (struct sockaddr){0};
        addr_len = 0;

        // int rv = recvfrom(SOCK_FD, buf, len, 0, &src_addr, &addr_len);

        if (rv <= 0)
        {
            printf("*(%d: %s) ", rv, strerror(errno));
            fflush(stdout);
        }
        else
        {
            recvfrom(SOCK_FD, buf, sizeof(buf), MSG_DONTWAIT, &src_addr, &addr_len);
            presentable(((struct sockaddr_in*)&src_addr)->sin_addr);
            printf("%s ", g_data.presentable);
        }
    }
}


void    loop()
{
    int ttl;

    presentable(g_data.dinfo.sin->sin_addr);
    printf("traceroute to %s (%s), %d hops max\n", 
        g_data.target, 
        g_data.presentable, 
        g_data.options.m
    );

    ttl = 1;
    for (int nqueries = 0; nqueries < g_data.options.m; nqueries++)
    {
        setsockopt(SOCK_FD, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
        printf("% 3d   ", nqueries + 1);
        send_packets();
        receive_packets();
        printf("\n");
        ttl++;
    }
}

int main(int argc, char **argv)
{
    g_data.target = argv[1];
    g_data.options.m = 64;
    g_data.options.q = 3;
    resolve_destination();
    setup_socket();
    loop();
    return (0);
}