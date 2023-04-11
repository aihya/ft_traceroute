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

void    presentable(struct in_addr addr, char *buffer, size_t size)
{
    ft_bzero(buffer, size);
    inet_ntop(AF_INET, &addr, buffer, size);
}

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
    
    SOCK_FD = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (SOCK_FD == -1)
    {
        fprintf(stderr, "traceroute: socket: %s\n", strerror(errno));
        exit(errno);
    }

    // addr = (struct sockaddr_in){0};
    // addr.sin_family = AF_INET;
    // addr.sin_port = htons(20);
    // addr.sin_addr.s_addr = INADDR_ANY;

    // if (bind(SOCK_FD, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    // {
    //     fprintf(stderr, "traceroute: bind: %s\n", strerror(errno));
    //     exit(errno);
    // }

}


void    send_packets()
{
    int             rv;
    struct icmphdr	*icmp;

	ft_memset(g_data.packet, 0x00, sizeof(g_data.packet));
	icmp = (struct icmphdr *)g_data.packet;
	icmp->type = ICMP_ECHO;
	icmp->code = 0;
	icmp->un.echo.id = (uint16_t)getpid();
	icmp->un.echo.sequence = g_data.sequence;

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
        if (rv < 0)
        {
            fprintf(stderr, "traceroute: %s\n", strerror(errno));
            exit(errno);
        }
    }
}


double  tv_to_double(struct timeval tv)
{
    return (tv.tv_sec * 1000. + tv.tv_usec  / 1000.);
}


double  delta_t(struct timeval t1, struct timeval t2)
{
    return tv_to_double(t2) - tv_to_double(t1);
}


struct timeval  extract_data(char *packet)
{
    struct iphdr    *ip;
    struct icmphdr  *icmp;
    struct iphdr    *oip;
    struct icmphdr  *oicmp;
    struct timeval  tv;
    struct timeval  *ptv;

    ip = (struct iphdr *)(packet);
    if (ip->protocol == IPPROTO_ICMP && ip->version == IPVERSION)
    {
        icmp = (struct icmphdr *)(packet + (ip->ihl << 2));
        if (icmp->type == ICMP_ECHOREPLY && icmp->code == 0 && icmp->un.echo.id == (uint16_t)getpid())
            tv = *(struct timeval *)(icmp + 1);
        else
        {
            oip   = (struct iphdr *)(packet + (ip->ihl << 2) + sizeof(struct icmphdr));
	        oicmp = (struct icmphdr *)(packet + (ip->ihl << 2) + sizeof(struct icmphdr) + (oip->ihl << 2));
            ptv = (struct timeval *)(oicmp + 1);
            printf("%d\n", ptv->tv_sec);
            tv = *(struct timeval *)(oicmp+1);
            printf("sec: %d, usec: %d\n", ((struct timeval *)(oicmp+1))->tv_sec, ((struct timeval *)(oicmp+1))->tv_usec);
        }
    }
    else
        tv = (struct timeval){0};
    printf("\n--> %d\n", tv);
    return tv;
}


int receive_packets()
{
    int             rv;
    char            buf[IP_MAXPACKET];
    struct sockaddr src_addr;
    socklen_t       addr_len;
    struct timeval  wait;
    struct timeval  send_tv;
    struct timeval  recv_tv;
    int             is_there;
    char            str[1024];
    static int      nqueries = 1;
    char            nqueries_str[6];
    int             new_line;

    new_line = 1;
    is_there = 0;
    for (int npackets = 0; npackets < g_data.options.q; npackets++)
    {
        ft_bzero(buf, sizeof(buf));

        FD_ZERO(&g_data.socket.readfds);
        FD_SET(SOCK_FD, &g_data.socket.readfds);

        wait.tv_sec = 3;
        wait.tv_usec = 0;
        rv = select(SOCK_FD + 1, &g_data.socket.readfds, NULL, NULL, &wait);
        gettimeofday(&recv_tv, NULL);

        if (rv <= 0)
            sprintf(str, " * ");
        else
        {
            src_addr = (struct sockaddr){0};
            addr_len = sizeof(struct sockaddr_in);
            rv = recvfrom(SOCK_FD, buf, sizeof(buf), MSG_WAITALL, &src_addr, &addr_len);
            send_tv = extract_data(buf);
            // if (send_tv.tv_sec == 0 && send_tv.tv_usec == 0)
                // printf("Mok momkina\n");
            struct sockaddr_in  sin = *(struct sockaddr_in *)&src_addr;
            presentable(sin.sin_addr, g_data.presentable.current, PRESENT_SIZE);
            sprintf(str, " %s  %.3lfms ", g_data.presentable.current, delta_t(send_tv, recv_tv));
        }

        if (!ft_strcmp(g_data.presentable.current, g_data.presentable.target))
            is_there = 1;

        if (new_line)
        {
            sprintf(nqueries_str, "% 3d  ", nqueries);
            ft_putstr(nqueries_str);
            new_line = 0;
        }

        ft_putstr(str);
    }
    nqueries++;
    return (is_there);
}


void    loop()
{
    int     ttl;

    presentable(g_data.dinfo.sin->sin_addr, g_data.presentable.target, PRESENT_SIZE);
    printf("traceroute to %s (%s), %lld hops max\n", 
        g_data.target, 
        g_data.presentable.target, 
        g_data.options.m
    );

    ttl = 1;
    for (int nqueries = 0; nqueries < g_data.options.m; nqueries++)
    {
        setsockopt(SOCK_FD, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
        send_packets();
        if (receive_packets())
        {
            printf("\n");
            break;
        }
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