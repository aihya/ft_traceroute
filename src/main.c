#include "ft_traceroute.h"

t_traceroute g_data = (t_traceroute){0};

void    usage(int __exit, int exit_code)
{
    printf(
"Usage: traceroute [-fnwqI] [-f first_ttl] \
[-q nqueries] [-n numeric_only] [-q number_probs] [-I icmp_protocol]\
[-w waittime]\n");
    if (__exit)
        exit(exit_code);
}


double	get_time_diff(struct timeval *stime, struct timeval *rtime)
{
	double	time;

	time = (rtime->tv_sec - stime->tv_sec) * 1000.;
	time += (rtime->tv_usec - stime->tv_usec) / 1000.;
	return (time);
}


char    *ascii(struct sockaddr *sa)
{
    return (inet_ntoa(((struct sockaddr_in *)sa)->sin_addr));
}


void    parse_options(int argc, char **argv)
{
    int         i;

    g_data.options = (t_options){0};
    g_data.options.f = 1;
    g_data.options.n = 0;
    g_data.options.w = 1;
    g_data.options.p = 33434;
    g_data.options.q = 3;
    g_data.options.I = 0;
    i = 0;
	while (i < argc)
	{
		if (!ft_strcmp("-h", argv[i]))
			usage(true, 0);
		else if (!ft_strcmp("-n", argv[i]))
			g_data.options.n = 1;
        else if (!ft_strcmp("-I", argv[i]))
			g_data.options.I = 1;
		else if (!ft_strcmp("-f", argv[i]))
		{
			if (i+1 < argc)
			{
				long long int	value = ft_atoll(argv[i+1]);
				if (value < 1 || value > 64)
				{
					fprintf(stderr, ": first hop out of range\n");
					exit(1);
				}
				g_data.options.f = ft_atoi(argv[i+1]);
				i += 2;
				continue;
			}
			printf("ping: option requires an argument '%s'\n", argv[i]);
			usage(true, 1);
		}
        else if (!ft_strcmp("-w", argv[i]))
		{
			if (i+1 < argc)
			{
				long long int	value = ft_atoll(argv[i+1]);
				if (value < 0 || value > 1000)
				{
					fprintf(stderr, "traceroute : out of range: 1 <= value <= 1000\n");
					exit(1);
				}
				g_data.options.w = ft_atoi(argv[i+1]);
				i += 2;
				continue;
			}
			printf("ping: option requires an argument '%s'\n", argv[i]);
			usage(true, 1);
		}
        else if (!ft_strcmp("-q", argv[i]))
		{
			if (i+1 < argc)
			{
				long long int	value = ft_atoll(argv[i+1]);
				if (value < 1 || value > 10)
				{
					fprintf(stderr, "traceroute: no more than 10 hops\n");
					exit(1);
				}
				g_data.options.q = ft_atoi(argv[i+1]);
				i += 2;
				continue;
			}
			printf("ping: option requires an argument '%s'\n", argv[i]);
			usage(true, 1);
		}
		else if (argv[i][0] == '-')
		{
			fprintf(stderr, "ping: invalid option -- '%s'\n", argv[i]);
			usage(true, 1);
		}
		else
        {
			g_data.target = argv[i];
        }
		i++;
	}
    if (!g_data.options.I)
    {
        g_data.type     = SOCK_DGRAM;
        g_data.protocol = IPPROTO_UDP;
    }
    else
    {
        g_data.type     = SOCK_RAW;
        g_data.protocol = IPPROTO_ICMP;
    }
}


void    update_ttl(int ttl)
{
    int ret;

    ret = setsockopt(g_data.socket.sfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
    if (ret < 0)
    {
        fprintf(stderr, "Failed update IP_TTL value.\n");
        freeaddrinfo(g_data.dinfo.result);
        exit(0);
    }
}


void    set_port(struct sockaddr *sa)
{
    struct sockaddr_in  *sin;

    sin = (struct sockaddr_in *)sa;
    sin->sin_port = htons(g_data.options.p++);
}


void    create_sockets()
{
    struct timeval  timeout;
    int             ret;

    g_data.socket.rfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (g_data.socket.rfd < 0)
    {
        fprintf(stderr, "failed to create receive socket\n");
        freeaddrinfo(g_data.dinfo.result);
        exit(-1);
    }

    g_data.socket.sfd = socket(AF_INET, g_data.type, g_data.protocol);
    if (g_data.socket.sfd < 0)
    {
        fprintf(stderr, "failed to create send socket\n");
        freeaddrinfo(g_data.dinfo.result);
        exit(-1);
    }

    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    ret = setsockopt(g_data.socket.rfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval));
    if (ret < 0)
    {
        fprintf(stderr, "Failed to set socket option: SO_RCVTIMEO\n");
        freeaddrinfo(g_data.dinfo.result);
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

    pkt.size = sizeof(struct icmp) + 52;
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

    pkt.size = sizeof(struct udphdr) + 52;
    ft_bzero(pkt.buff, pkt.size);
    set_port(&g_data.sa.sa_send);
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
    gettimeofday(&g_data.send_time, NULL);
}


int     recv_packet(int is_last)
{
    char            buffer[1024];
    char            buff[256];
    struct sockaddr address;
    socklen_t       address_len;
    ssize_t         ret;
    struct ip       *ip;
    struct icmphdr  *icmp;
    struct sockaddr_in *sin;

    ft_bzero(buffer, sizeof(buffer));
    address = (struct sockaddr){0};
    address_len = sizeof(address);
    ret = recvfrom(g_data.socket.rfd, buffer, sizeof(buffer), 0, &address, &address_len);
    gettimeofday(&g_data.recv_time, NULL);
    if (ret >= 0)
    {
        ip = (struct ip *)buffer;
        icmp = (struct icmphdr *)(buffer + (ip->ip_hl << 2));
        ret = 0;
        if ((g_data.protocol == IPPROTO_ICMP &&icmp->type == 0) || (g_data.protocol == IPPROTO_UDP && icmp->type == 3 && icmp->code == 3))
            ret = 1;
        if (g_data.last_addr.s_addr != ((struct sockaddr_in *)&address)->sin_addr.s_addr)
        {
            if (!g_data.options.n)
            {
                ft_bzero(buff, sizeof(buff));
                getnameinfo(&address, address_len, buff, sizeof(buff), NULL, 0, 0);
                ft_putstr(buff);
                ft_putstr(" (");
            }
            ft_putstr(ascii(&address));
            if (!g_data.options.n)
                ft_putchar(')');
            ft_putstr("  ");
        }
        
        g_data.last_addr.s_addr = (((struct sockaddr_in *)&address)->sin_addr).s_addr;
        
        ft_bzero(buff, sizeof(buff));
        sprintf(buff, "%.3f ms", get_time_diff(&g_data.send_time, &g_data.recv_time));
        ft_putstr(buff);
        if (!is_last)
            ft_putstr("  "); 
    }
    else
    {
        ft_putstr("*");
        if (!is_last)
            ft_putchar(' ');
        ret = 0;
    }
    return (ret);
}


void    traceroute_loop()
{
    int i_f;
    int i_q;
    int hit;

    i_f = g_data.options.f;
    while (i_f < 64)
    {
        if (i_f < 10)
            ft_putchar(' ');
        update_ttl(i_f);
        ft_putnbr(i_f);
        ft_putstr("  ");
        hit = 0;
        i_q = 0;
        g_data.last_addr = (struct in_addr){0};
        while (i_q < g_data.options.q)
        {
            send_packet();
            hit = recv_packet(i_q+1 == g_data.options.q);
            i_q++;
        }
        ft_putchar('\n');
        if (hit)
            break ;
        i_f++;
    }
}


void    resolve_target()
{
    struct addrinfo     ai;
    struct addrinfo     hints;
    struct addrinfo     *ptr;
    int                 ret;
    struct addrinfo     *target;
    struct sockaddr_in  *sin;

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
            }
        }
        if (target)
        {
            printf("tracerouet to %s (%s), 64 hops max, 60 byte packet \n", g_data.target, ascii(target->ai_addr));
            g_data.dinfo.ai = *target;
            g_data.sa.sa_send = *g_data.dinfo.ai.ai_addr;
        }
    }
    else if (ret < 0)
    {
        fprintf(stderr, "traceroute: %s\n", gai_strerror(ret));
        exit(ret);
    }
}


int     main(int argc, char **argv)
{
    if (argc == 1)
        usage(true, 1);
    parse_options(argc - 1, argv + 1);
    if (g_data.target == NULL)
    {
        fprintf(stderr, "missing host argument.\n");
        exit(1);
    }
    resolve_target();
    create_sockets();
    traceroute_loop();
    freeaddrinfo(g_data.dinfo.result);
    return (0);
}