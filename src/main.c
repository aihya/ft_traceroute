#include "ft_traceroute.h"

t_traceroute g_data = (t_traceroute){0};

void    usage()
{
    printf(
"Usage: traceroute [-adDeFInrSvx] [-f first_ttl] \
[-M first_ttl] [-m max_ttl] [-P proto] [-q nqueries] [-s src_addr] \
[-w waittime] [-z pausemsecs] host [packetlen]\n");
}

void    parse_options(int argc, char **argv)
{
    int         i;

    g_data.options = (t_options){0};
    g_data.options.M = 1;
    g_data.options.q = 3;
    g_data.options.n = 0;
    g_data.options.m = 64;
    g_data.options.S = 0;
    g_data.options.w = 5;
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

void    create_socket()
{
    g_data.socket.fd = 
}

void    send_packet()
{

}

void    recv_packet()
{

}

void    traceroute_loop()
{
    int i_m;
    int i_q;

    i_m = 0;
    while (i_m < g_data.options.m)
    {
        i_q = 0;
        while (i_q < g_data.options.q)
        {
            send_packet();
            recv_packet();
            i_q++;
        }
        i_m++;
    }
}

int     resolve_target()
{
    struct addrinfo ai;
    struct addrinfo hints;
    struct addrinfo *ptr;
    int             ret;

    ft_bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_ICMP;
    ret = getaddrinfo(g_data.target, NULL, &hints, &g_data.dinfo.result);
    if (ret == 0)
    {
        for (ptr = g_data.dinfo.result; ptr; ptr = ptr->ai_next)
        {
            if (ptr->ai_family == AF_INET
             && ptr->ai_protocol == IPPROTO_ICMP
             && ptr->ai_socktype == SOCK_RAW)
                break ;
        }
        if (ptr)
            g_data.dinfo.ai = *ptr;
    }
    return (ret);
}

int     main(int argc, char **argv)
{
    parse_options(argc - 1, argv + 1);
    g_data.target = argv[1];
    if (resolve_target() != 0)
    {
        printf("Mok\n");
        exit(0);
    }
    traceroute_loop();
    return (0);
}