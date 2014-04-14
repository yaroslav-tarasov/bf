#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <linux/if_ether.h>
#include <linux/netlink.h>
#include <string.h>
#include "utils.h"

#include "trx_data.h"
namespace cmd_parse {
int
get_proto(char* proto) {
    if (strcmp(proto, "ALL") == 0) {
        return IPPROTO_NOTEXIST;
    } else if (strcmp(proto, "TCP") == 0) {
        return IPPROTO_TCP;
    } else if (strcmp(proto, "UDP") == 0) {
        return IPPROTO_UDP;
    }
    return -1;
}

inline 	const char*
get_proto_name(int proto) {
    static const char* proto_names[]=
    {"ALL","TCP","UDP"};
    if (IPPROTO_NOTEXIST == proto) {
        return proto_names[0];
    } else if (IPPROTO_TCP == proto ) {
        return proto_names[1];
    } else if (IPPROTO_UDP == proto ) {
        return proto_names[2];
    }
    return NULL;
}

int
get_direction(char* dir) {
    if (strcmp(dir, "ALL") == 0) {
        return DIR_ALL;
    } else if (strcmp(dir, "INPUT") == 0) {
        return DIR_INPUT;
    } else if (strcmp(dir, "OUTPUT") == 0) {
        return DIR_OUTPUT;
    }
}


int
get_policy(char* policy) {
    if (strcmp(policy, "DROP") == 0) {
    return POLICY_DROP;
    } else if (strcmp(policy, "ACCEPT") == 0) {
    return POLICY_ACCEPT;
    }
}

int
parse_cmd_args(int argc, char *argv[],filter_rule_t* td)
{
    int c; int command = CMD_PRINT_HELP;
    int tmp_in_out = 0;
    struct in_addr ipvalue;

    memset(td,0,sizeof(filter_rule_t));

    while (1)
    {
        static const struct option long_options[] =
        {
            {"list", required_argument, NULL, 'L'},
            {"new", required_argument, NULL, 'N'},
            {"delete", required_argument, NULL, 'D'},
            {"srcip", required_argument, NULL, 's'},
            {"srcnetmask", required_argument, NULL, 'm'},
            {"srcport", required_argument, NULL, 'p'},
            {"destip", required_argument, NULL, 't'},
            {"destnetmask", required_argument, NULL, 'n'},
            {"destport", required_argument, NULL, 'd'},
            {"proto", required_argument, NULL, 'c'},
            {"action", required_argument, NULL, 'P'},
            {0, 0, 0, 0}
        };
        int option_index = 0;
        c = getopt_long(argc, argv, "L:N:D:s:m:p:t:n:d:c:P", long_options, &option_index);
        /*Detect the end of the options. */
        if (c == -1)
            break;
        //command = CMD_PRINT_HELP;
        switch (c)
        {
            case 0:
              //printf("flag option: %s, mf_rule.in_out = %d\n", long_options[option_index].name, mf_rule.in_out);
              break;
            case 'L':
                command = CMD_PRINT_RULES;
        td->direction = get_direction(optarg);
        printf("direction in_out = %d\n", td->direction);
              break;
            case 'N':
                command = CMD_NEW_RULE;
        td->direction = get_direction(optarg);
        printf("direction in_out = %d\n", td->direction);
              break;
            case 'D':
              command = CMD_DEL_RULE;       //delete
          td->direction = get_direction(optarg);
          printf("direction in_out = %d\n", td->direction);
              break;
            case 's':
              //mf_rule.src_ip = optarg;  //src ip

          {
          int s = inet_pton(AF_INET, optarg, &ipvalue);
               switch(s) {
               case 1:
              printf("converted value = %x \n", ipvalue.s_addr);
              td->base_rule.s_addr.addr = ipvalue.s_addr;
              break;
               case 0:
              printf("invalid input: %s\n", optarg);
              break;
               default:
              printf("inet_pton conversion error \n");
              break;
                }
          }
          break;
            case 'm':
              //mf_rule.src_netmask = optarg; //srcnetmask:
              break;
            case 'p':
              td->base_rule.src_port = atoi(optarg);    //srcport:
              break;
            case 't':
          {
          printf("converted value  d_addr.addr\n");
          int s = inet_pton(AF_INET, optarg, &ipvalue);
               switch(s) {
               case 1:
              printf("converted value = %x \n", ipvalue.s_addr);
              td->base_rule.d_addr.addr = ipvalue.s_addr;
              break;
               case 0:
              printf("invalid input: %s\n", optarg);
              break;
               default:
              printf("inet_pton conversion error \n");
              break;
                }
          }
              break;
            case 'n':
              //mf_rule.dest_netmask = optarg;    //destnetmask
              break;
            case 'd':
              td->base_rule.dst_port = atoi(optarg);    //destport
              break;
            case 'c':
              td->base_rule.proto = get_proto(optarg); //proto
              if (td->base_rule.proto==-1)
        command = CMD_PRINT_HELP;
              break;
            case 'P':
               td->policy = get_policy(optarg);
              break;
            case '?':
              /* getopt_long printed an error message. */
              break;
            default:
              abort();
        }
    }

/*    if (optind < argc)
    {
        //printf("non-option ARGV-elements: ");
        while (optind < argc)
        //printf("%s ", argv[optind++]);
        putchar('\n');
    }
*/



   return command;
}

}
