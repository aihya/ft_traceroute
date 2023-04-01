#include "ft_traceroute.h"

t_traceroute g_data = (t_traceroute){0};

void    resolve_destination()
{
    int             ret;
    struct addrinfo hints;
    struct addrinfo *p;

    hints = (struct addrinfo){0};
    hints.ai_family     = AF_INET;
    hints.ai_socktype   = SOCK_RAW;
    hints.ai_protocol   = IPPROTO_ICMP;

    if ((ret = getaddrinfo(g_data.target, NULL, &hints, &g_data.dinfo.result)) < 0)
    {
        fprintf(stderr, "traceroute: %s: %s\n", g_data.target, gai_strerror(ret));
        exit(ret);
    }

    for (p = g_data.dinfo.result; p; p = p->ai_next)
    {
        if (p->ai_protocol == IPPROTO_ICMP && p->ai_family == AF_INET)
            break;
    }
    if (p)
    {
        // TODO: IMPORTANT: Replace with ft_memcpy
        ft_memcpy(&g_data.dinfo.ai, p, sizeof(struct addrinfo));
        g_data.dinfo.sa = g_data.dinfo.ai.ai_addr;
        g_data.dinfo.sin = (struct sockaddr_in *)g_data.dinfo.sa;
        return ;
    }
    exit(0);
}


void    setup_socket()
{
    g_data.socket.fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_data.socket.fd == -1)
    {
        fprintf(stderr, "traceroute: socket: %s\n", strerror(errno));
        exit(errno);
    }

    FD_SET(g_data.socket.fd, &g_data.socket.read_set);
}


void    send_packet()
{
    struct icmphdr	*icmp;

	ft_memset(g_data.packet.buff, 0x00, sizeof(g_data.packet.buff));
	icmp = (struct icmphdr *)g_data.packet.buff;
	icmp->type = ICMP_ECHO;
	icmp->code = 0;
	icmp->un.echo.id = (uint16_t)getpid();
	icmp->un.echo.sequence = g_data.sequence;
	gettimeofday((void *)(icmp + 1), 0);
	icmp->checksum = 0;
	icmp->checksum = calc_checksum((uint16_t *)icmp, sizeof(g_data.packet));
    
    sendto(g_data.socket.fd, g_data.packet.buff, sizeof(g_data.packet.buff), );
}


void    receive_packet()
{

}


void    traceroute()
{
    int             nqueries;
    int             npackets;
    int             rv;
    struct timeval  tv;

    tv.tv_sec = 10;
    tv.tv_usec = 0;
    FD_ZERO(&g_data.socket.send_set);
    FD_ZERO(&g_data.socket.read_set);
    FD_ZERO(&g_data.socket.expt_set);

    nqueries = 0;
    while (nqueries < g_data.options.m)
    {
        // Send query packets.
        npackets = 0;
        while (npackets < g_data.options.q)
        {
            printf("sending...\n");
            // while ((rv = select(1, NULL, &g_data.socket.send_set, NULL, NULL)) == 0)
            //     ;
            send_packet();
            printf("ttl: %llu sent: %llu\n", nqueries, npackets);
            npackets++;
        }

        // Receive query packets.
        npackets = 0;
        while (npackets < g_data.options.q)
        {
            printf("receiving...\n");
            while ((rv = select(1, &g_data.socket.read_set, NULL, NULL, &tv)) == 0)
                ;
            receive_packet();
            printf("ttl: %llu sent: %llu\n", nqueries, npackets);
            npackets++;
        }

        nqueries++;
    }
}

int main(int argc, char **argv)
{
    g_data.target = argv[1];
    g_data.options.m = 30;
    g_data.options.q = 3;
    resolve_destination();
    setup_socket();
    traceroute();
    return (0);
}