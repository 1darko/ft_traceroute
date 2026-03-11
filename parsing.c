#include "traceroute.h"

void value_error(const char *value, char near_char, int type){
    fprintf(stderr, "ft_traceroute: invalid value (`%s' near `%c')\n", type ? value + 2 : value, near_char);
    fprintf(stderr, "Try 'ft_traceroute --help' for more information.\n");
};

int value_check(traceroute_options *opts)
{
    // if(opts->q_probes <= 0 || opts->s_value > 1473){
    //     fprintf(stderr, "ft_traceroute: invalid payload size '%d'\n", opts->s_value);
    //     return 1;
    // }
    if(opts->q_probes && (atoi(opts->q_probes) <= 0 || int_overflow(opts->q_probes))){
        fprintf(stderr, "ft_traceroute: invalid probes '%s'\n", opts->q_probes);
        return 1;
    }
    if(opts->wait_between_probes && (atoi(opts->wait_between_probes) <= 0 || int_overflow(opts->wait_between_probes))){
        fprintf(stderr, "ft_traceroute: invalid wait time '%s'\n", opts->wait_between_probes);
        return 1;
    }
    if(opts->timeout && (atoi(opts->timeout) < 0 || int_overflow(opts->timeout))){
        fprintf(stderr, "ft_traceroute: invalid timeout '%s'\n", opts->timeout);
        return 1;
    }
    if(opts->first_ttl && (atoi(opts->first_ttl) < 1 || int_overflow(opts->first_ttl))){
        fprintf(stderr, "ft_traceroute: invalid first TTL '%s'\n", opts->first_ttl);
        return 1;
    }
    // if(opts->starting_port && port_range(opts->starting_port)){
    //     fprintf(stderr, "ft_traceroute: invalid starting port '%s'\n", opts->starting_port);
    //     return 1;
    // }
    if(opts->max_hops && (atoi(opts->max_hops) < 1 || int_overflow(opts->max_hops))){
        fprintf(stderr, "ft_traceroute: invalid max hops '%s'\n", opts->max_hops);
        return 1;
    }
    return 0;
}

static int parse_value_option(int *i, int ac, char **av, int j, char opt, char **dest)
{
    if (av[*i][j + 1] != '\0') {
        if (!isnumeric(&av[*i][j + 1]))
            return (value_error(av[*i], opt, 1), 1);
        *dest = &av[*i][j + 1];
    }
    else if (++(*i) < ac && av[*i][0] != '\0') {
        if (!isnumeric(av[*i]))
            return (value_error(av[*i], opt, 0), 1);
        *dest = av[*i];
    }
    else {
        fprintf(stderr, "ft_traceroute: option '-%c' requires an argument\n", opt);
        return 1;
    }
    return 0;
}

int parser(int ac, char **av, traceroute_options *opts, char **dst_ip)
{
    int break_flag;
    int ip_found = 0;

    for (int i = 1; i < ac; i++) {
        break_flag = 0;
        if (av[i][0] == '-') {
            for (int j = 1; av[i][j] != '\0' && !break_flag; j++) {
                switch (av[i][j]) {
                    case '-':
                        ft_strcmp(av[i], "--help") == 0 ? print_help() : print_invalid_option(av[i]);
                        return 1;
                    case 'q': if (parse_value_option(&i, ac, av, j, 'q', &opts->q_probes))            return 1; break_flag = 1; break;
                    case 'z': if (parse_value_option(&i, ac, av, j, 'z', &opts->wait_between_probes)) return 1; break_flag = 1; break;
                    case 'w': if (parse_value_option(&i, ac, av, j, 'w', &opts->timeout))             return 1; break_flag = 1; break;
                    case 'm': if (parse_value_option(&i, ac, av, j, 'm', &opts->max_hops))            return 1; break_flag = 1; break;
                    case 'p': if (parse_value_option(&i, ac, av, j, 'p', &opts->starting_port))       return 1; break_flag = 1; break;
                    case 'f': if (parse_value_option(&i, ac, av, j, 'f', &opts->first_ttl))           return 1; break_flag = 1; break;
                    default:
                        fprintf(stderr, "ft_traceroute: invalid option -- '%c'\n", av[i][j]);
                        fprintf(stderr, "Try 'ft_traceroute --help' for more information.\n");
                        return 1;
                }
            }
        }
        else if (ip_found == 0) {
            *dst_ip = av[i];
            ip_found = 1;
        }
        else {
            fprintf(stderr, "ft_traceroute: extra argument '%s'\n", av[i]);
            fprintf(stderr, "Try 'ft_traceroute --help' for more information.\n");
            return 1;
        }
    }
    if (!ip_found) {
        fprintf(stderr, "ft_traceroute: destination address required\n");
        fprintf(stderr, "Try 'ft_traceroute --help' for more information.\n");
        return 1;
    }
    if (value_check(opts))
        return 1;
    opts->q_probes_value              = opts->q_probes            ? atoi(opts->q_probes)           : PROBES_TO_SEND;
    opts->timeout_value               = opts->timeout             ? atoi(opts->timeout)            : TIMEOUT;
    opts->max_hops_value              = opts->max_hops            ? atoi(opts->max_hops)           : MAX_HOPS;
    opts->first_ttl_value             = opts->first_ttl           ? atoi(opts->first_ttl)          : 1;
    if (opts->wait_between_probes)
        opts->wait_between_probes_value = atoi(opts->wait_between_probes);
    if (!opts->starting_port) {
        opts->starting_port_value = STARTING_PORT;
        opts->starting_port_set   = 1;
    } else {
        opts->starting_port_value = atoi(opts->starting_port);
        if (opts->starting_port_value == 0) {
            fprintf(stderr, "ft_traceroute: invalid starting port '%s'\n", opts->starting_port);
            return 1;
        }
        opts->starting_port_set = 0;
    }
    return 0;
}

// int parser(int ac, char **av, traceroute_options *opts, char **dst_ip)
// {
//     int break_flag;
//     int ip_found = 0;
//     for(int i = 1; i < ac; i++){
//         break_flag = 0;
//         if(av[i][0] == '-'){
//             for(int j = 1; av[i][j] != '\0' && !break_flag; j++){
//                 switch(av[i][j]){
//                     case '-':
//                         ft_strcmp(av[i], "--help") == 0 ? print_help() : print_invalid_option(av[i]);
//                         return 1;
//                     case 'q':
//                         if(av[i][j + 1] != '\0'){
//                             if(!isnumeric(&av[i][j + 1])){
//                                 return (value_error(av[i], 'q', 1), 1);
//                             }
//                             opts->q_probes = &av[i][j + 1];
//                             break_flag = 1;
//                             break;
//                         }
//                         else if(++i < ac && av[i][0] != '\0') {
//                             if(!isnumeric(av[i])){
//                                 return (value_error(av[i], 'q', 0), 1);
//                             }
//                             opts->q_probes = av[i];
//                         }
//                         else{
//                             fprintf(stderr, "ft_traceroute: option '-q' requires an argument\n");
//                             return 1;
//                         }
//                         break_flag = 1;
//                         break;
//                     case 'z':
//                         if(av[i][j + 1] != '\0'){
//                             if(!isnumeric(&av[i][j + 1])){
//                                 return (value_error(av[i], 'z', 1), 1);
//                             }
//                             opts->wait_between_probes = &av[i][j + 1];
//                         }
//                         else if(++i < ac && av[i][0] != '\0') {
//                             if(!isnumeric(av[i])){
//                                 return (value_error(av[i], 'z', 0), 1);
//                             }
//                             opts->wait_between_probes = av[i];
//                         }
//                         else{
//                             fprintf(stderr, "ft_traceroute: option '-z' requires an argument\n");
//                             return 1;
//                         }
//                         break_flag = 1;
//                         break;
//                     case 'w':
//                         if(av[i][j + 1] != '\0'){
//                             if(!isnumeric(&av[i][j + 1])){
//                                 return (value_error(av[i], 'w', 1), 1);
//                             }
//                             opts->timeout = &av[i][j + 1];
//                         }
//                         else if(++i < ac && av[i][0] != '\0') {
//                             if(!isnumeric(av[i])){
//                                 return (value_error(av[i], 'w', 0), 1);
//                             }
//                             opts->timeout = av[i];
//                         }
//                         else{
//                             fprintf(stderr, "ft_traceroute: option '-w' requires an argument\n");
//                             return 1;
//                         }
//                         break_flag = 1;
//                         break;
//                     case 'm':
//                         if(av[i][j + 1] != '\0'){
//                             if(!isnumeric(&av[i][j + 1])){
//                                 return (value_error(av[i], 'm', 1), 1);
//                             }
//                             opts->max_hops = &av[i][j + 1];
//                         }
//                         else if(++i < ac && av[i][0] != '\0') {
//                             if(!isnumeric(av[i])){
//                                 return (value_error(av[i], 'm', 0), 1);
//                             }
//                             opts->max_hops = av[i];
//                         }
//                         else{
//                             fprintf(stderr, "ft_traceroute: option '-m' requires an argument\n");
//                             return 1;
//                         }
//                         break_flag = 1;
//                         break;
//                     case 'p':
//                         if(av[i][j + 1] != '\0'){
//                             if(!isnumeric(&av[i][j + 1])){
//                                 return (value_error(av[i], 'p', 1), 1);
//                             }
//                             opts->starting_port = &av[i][j + 1];
//                         }
//                         else if(++i < ac && av[i][0] != '\0') {
//                             if(!isnumeric(av[i])){
//                                 return (value_error(av[i], 'p', 0), 1);
//                             }
//                             opts->starting_port = av[i];
//                         }
//                         else{
//                             fprintf(stderr, "ft_traceroute: option '-p' requires an argument\n");
//                             return 1;
//                         }
//                         break_flag = 1;
//                         break;
//                     case 'f':
//                         if(av[i][j + 1] != '\0'){
//                             if(!isnumeric(&av[i][j + 1])){
//                                 return (value_error(av[i], 'f', 1), 1);
//                             }
//                             opts->first_ttl = &av[i][j + 1];
//                         }
//                         else if(++i < ac && av[i][0] != '\0') {
//                             if(!isnumeric(av[i])){
//                                 return (value_error(av[i], 'f', 0), 1);
//                             }
//                             opts->first_ttl = av[i];
//                         }
//                         else{
//                             fprintf(stderr, "ft_traceroute: option '-f' requires an argument\n");
//                             return 1;
//                         }
//                         break_flag = 1;
//                         break;
//                     default:
//                         fprintf(stderr, "ft_traceroute: invalid option -- '%c'\n", av[i][j]);
//                         fprintf(stderr, "Try 'ft_traceroute --help' for more information.\n");
//                         return 1;
//                     }
//                 }
//             }
//             else if(ip_found == 0){
//                 *dst_ip = av[i];
//                 ip_found = 1;
//             }
//             else{
//                 fprintf(stderr, "ft_traceroute: extra argument '%s'\n", av[i]);
//                 fprintf(stderr, "Try 'ft_traceroute --help' for more information.\n");
//                 return 1;
//             }
//         }
//         if(!ip_found){
//             fprintf(stderr, "ft_traceroute: destination address required\n");
//             fprintf(stderr, "Try 'ft_traceroute --help' for more information.\n");
//             return 1;
//     }
//     if(value_check(opts))
//         return 1;
//     !opts->q_probes ? (opts->q_probes_value = PROBES_TO_SEND) : (opts->q_probes_value = atoi(opts->q_probes));
//     if(opts->wait_between_probes)
//         opts->wait_between_probes_value = atoi(opts->wait_between_probes);
//     !opts->timeout ? (opts->timeout_value = TIMEOUT) : (opts->timeout_value = atoi(opts->timeout));
//     !opts->max_hops ? (opts->max_hops_value = MAX_HOPS) : (opts->max_hops_value = atoi(opts->max_hops));
//     !opts->first_ttl ? (opts->first_ttl_value = 1) : (opts->first_ttl_value = atoi(opts->first_ttl));
//     if(!opts->starting_port){
//         opts->starting_port_value = STARTING_PORT;
//         opts->starting_port_set = 1;
//     }
//     else{
//         opts->starting_port_value = atoi(opts->starting_port);
//         if(opts->starting_port_value == 0)
//         {
//             fprintf(stderr, "ft_traceroute: invalid starting port '%s'\n", opts->starting_port);
//             return 1;
//         }
//         opts->starting_port_set = 0;
//     }

//     return 0;
// };
