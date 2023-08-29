#include "ft_traceroute.h"

t_traceroute g_data = (t_traceroute){0};

void    usage()
{
    printf(
"Usage: traceroute [-adDeFInrSvx] [-f first_ttl] \
[-M first_ttl] [-m max_ttl] [-P proto] [-q nqueries] [-s src_addr] \
[-w waittime] [-z pausemsecs] host [packetlen]\n");
}


char    *ascii(struct sockaddr *sa)
{
    return (inet_ntoa(((struct sockaddr_in *)sa)->sin_addr));
}


void    parse_options(int argc, char **argv)
{
    int         i;

    g_data.options = (t_options){0};
    g_data.options.M = 10;
    g_data.options.q = 3;
    g_data.options.n = 0;
    g_data.options.m = 64;
    g_data.options.w = 1;
    // i = 0;
    // while (i < argc)
    // {
    //     if (!ft_strcmp(argv[i], "-"))
    //         g_data.options.m = ;
    //     else if (!ft_begins_with(argv[i], "-"))
    //     {
    //         fprintf(stderr, "unrecognized option: `%s`", argv[i]);
    //         exit(0);
    //     }
    //     else
    //         g_data.target = argv[i];
    //     i++;
    // }
    // if (g_data.target == NULL)
    // {}
}


void    update_ttl(int ttl)
{
    int ret;

    ret = setsockopt(g_data.socket.sfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
    if (ret < 0)
    {
        fprintf(stderr, "Failed update IP_TTL value.\n");
        exit(0);
    }
}


void    set_port(int port, struct sockaddr *sa)
{
    struct sockaddr_in  *sin;

    sin = (struct sockaddr_in *)sa;
    sin->sin_port = htons(port);
}


void    create_sockets()
{
    struct timeval  timeout;
    int             ret;
    int             ttl;
    struct sockaddr_in  *sin;
    struct sockaddr_in  this_sin;
    int                 type;

    // Receive socket
    g_data.socket.rfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (g_data.socket.rfd < 0)
    {
        fprintf(stderr, "failed to create receive socket\n");
        exit(-1);
    }

    // Send socket
    g_data.socket.sfd = socket(AF_INET, g_data.type, g_data.protocol);
    if (g_data.socket.sfd < 0)
    {
        fprintf(stderr, "failed to create send socket\n");
        // Todo: Call free()
        exit(-1);
    }

    if (g_data.protocol == IPPROTO_UDP)
    {
        g_data.sa.sa_bind.sa_family = g_data.sa.sa_send.sa_family;
        set_port(33434, &g_data.sa.sa_send);
        bind(g_data.socket.sfd, &g_data.sa.sa_bind, g_data.dinfo.ai.ai_addrlen);
    }

    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    ret = setsockopt(g_data.socket.rfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval));
    if (ret < 0)
    {
        fprintf(stderr, "Failed to set socket option: SO_RCVTIMEO\n");
        exit(0);
    }
}

 
uint16_t    checksum(uint16_t *buffer, size_t size)
{
    uint32_t    result;
    int         count;

    result = 0;
    count = size;
    while (count)
    {
        result += *buffer++;
        count -= 2;
    }
    if (count)
        result += *(uint8_t *)buffer;
    while (result >> 16)
        result = (result >> 16) + (0xffff & result);
    return ((uint16_t)~result);
}


t_packet    *construct_icmp_packet()
{
    static t_packet pkt;
    struct icmphdr  *icmp;
    static ushort   sequence = 1;

    pkt.size = sizeof(struct icmp) + 54;
    ft_bzero(pkt.buff, pkt.size);
    icmp = (struct icmphdr *)pkt.buff;
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->un.echo.id = (ushort)getpid();
    icmp->un.echo.sequence = sequence++;
    icmp->checksum = 0;
    icmp->checksum = checksum((uint16_t *)pkt.buff, pkt.size);

    return (&pkt);
}


t_packet    *construct_udp_packet()
{
    static t_packet pkt;
    struct udphdr   *udp;

    pkt.size = sizeof(struct udphdr) + 54;
    ft_bzero(pkt.buff, pkt.size);
    udp = (struct udphdr *)pkt.buff;
    udp->uh_dport = ((struct sockaddr_in *)&g_data.sa.sa_send)->sin_port;
    udp->uh_sport = ((struct sockaddr_in *)&g_data.sa.sa_bind)->sin_port;
    udp->uh_ulen = htons(pkt.size);
    udp->uh_sum = 0;
    udp->uh_sum = checksum((uint16_t *)pkt.buff, pkt.size);
    return (&pkt);
}

void    send_packet()
{
    int             bytes;
    t_packet        *packet;

    if (g_data.protocol == IPPROTO_ICMP)
        packet = construct_icmp_packet();
    else
        packet = construct_udp_packet();
    bytes = sendto(
        g_data.socket.sfd, packet->buff, packet->size, 0, 
        &g_data.sa.sa_send, 
        g_data.dinfo.ai.ai_addrlen
    );
}


int     recv_packet()
{
    char            buffer[1024];
    struct sockaddr address;
    socklen_t       address_len;
    ssize_t          ret;
    uint16_t        cksum;
    struct ip       *ip;
    struct icmphdr  *icmp;
    struct hostent  *host;

    ft_bzero(buffer, sizeof(buffer));
    address = (struct sockaddr){0};
    address_len = sizeof(address);
    ret = recvfrom(g_data.socket.rfd, buffer, sizeof(buffer), 0, &address, &address_len);
    if (ret >= 0)
    {
        ip = (struct ip *)buffer;
        icmp = (struct icmphdr *)(buffer + (ip->ip_hl << 2));
        ret = 0;
        ft_putnbr(icmp->type);
        ft_putstr("-");
        ft_putnbr(icmp->code);
        ft_putstr("-");
        if (icmp->type == 0 || (icmp->type == 3 && icmp->code == 3))
            ret = 1;

        char buf[256] = {0};
        getnameinfo(&address, address_len, buf, sizeof(buf), NULL, 0, 0);
        
        ft_putstr(buf);
        ft_putstr("  ");

        // char *ptr = ascii(&address);
        // ft_putstr(ptr);
        // ft_putstr("  ");
    }
    else
    {
        ft_putstr("* ");
        ret = 0;
    }
    return (ret);
}


void    traceroute_loop()
{
    int i_m;
    int i_q;
    int hit;

    i_m = g_data.options.M;
    while (i_m < g_data.options.m)
    {
        update_ttl(i_m);
        ft_putnbr(i_m);
        ft_putstr("  ");
        hit = 0;
        i_q = 0;
        while (i_q < g_data.options.q)
        {
            send_packet();
            i_q++;
        }
        i_q = 0;
        while (i_q < g_data.options.q)
        {
            
            hit = recv_packet();
            i_q++;
        }
        ft_putchar('\n');
        if (hit)
            break ;
        i_m++;
    }
}


int     resolve_target()
{
    struct addrinfo ai;
    struct addrinfo hints;
    struct addrinfo *ptr;
    int             ret;
    int             count;
    struct addrinfo *target;
    struct sockaddr_in  *sin;

    count = 0;
    ft_bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = g_data.type;
    hints.ai_protocol = g_data.protocol;

    target = NULL;
    ret = getaddrinfo(g_data.target, NULL, &hints, &g_data.dinfo.result);
    if (ret == 0)
    {
        for (ptr = g_data.dinfo.result; ptr; ptr = ptr->ai_next)
        {
            if (ptr->ai_family == AF_INET && ptr->ai_protocol == g_data.protocol && ptr->ai_socktype == g_data.type)
            {
                if (!target)
                    target = ptr;
                count++;
            }
        }
        if (target)
        {
            if (count > 1)
                printf("traceroute: Warning: %s has multiple addresses; using %s\n", g_data.target, ascii(target->ai_addr));
            printf("tracerouet to %s (%s), %d hops max\n", g_data.target, ascii(target->ai_addr), g_data.options.m); // TODO: add the size of the packets

            g_data.dinfo.ai = *target;
            g_data.sa.sa_send = *g_data.dinfo.ai.ai_addr;
        }
    }
    return (ret);
}


int     main(int argc, char **argv)
{
    int type = 2;
    if (type == 1)
    {
        g_data.type     = SOCK_DGRAM;
        g_data.protocol = IPPROTO_UDP;
    }
    else
    {
        g_data.type     = SOCK_RAW;
        g_data.protocol = IPPROTO_ICMP;
    }

    parse_options(argc - 1, argv + 1);
    g_data.target = argv[1];
    if (resolve_target() != 0)
        exit(0);
    create_sockets();
    traceroute_loop();
    return (0);
}