#include "ft_traceroute.h"

t_options   parse_options(int argc, char **argv)
{
    int         i;
    t_options   options;

    options = (t_options){0};
    i = 0;
    while (i < argc)
    {
        if (!ft_strcmp(argv[i], "-"))
            options.resolve = 1;
        else if (!ft_begins_with(argv[i], "-"))
        {
            fprintf(stderr, "unrecognized option: `%s`", argv[i]);
            exit(0);
        }
        else
            g_data.target = argv[i];
        i++;
    }
}