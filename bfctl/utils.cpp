#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <linux/if_ether.h>
#include <linux/netlink.h>
#include <string.h>
#include <iostream>

#include "trx_data.h"
#include "utils.h"

namespace cmd_utils {
int
get_proto(char* proto) {
    if (strcmp(proto, "ALL") == 0) {
        return IPPROTO_ALL;
    } else if (strcmp(proto, "TCP") == 0) {
        return IPPROTO_TCP;
    } else if (strcmp(proto, "UDP") == 0) {
        return IPPROTO_UDP;
    }
    return IPPROTO_NOTEXIST;
}

int
get_switch(char* off) {
    if (strcmp(off, "NO") == 0) {
        return SW_NO;
    } else if (strcmp(off, "YES") == 0) {
        return SW_YES;
    }

    return SW_NONE;
}

int
get_chain(char* dir) {
    if (strcmp(dir, "ALL") == 0) {
        return CHAIN_ALL;
    } else if (strcmp(dir, "INPUT") == 0) {
        return CHAIN_INPUT;
    } else if (strcmp(dir, "OUTPUT") == 0) {
        return CHAIN_OUTPUT;
    }
    return CHAIN_NONE;
}



int
get_policy(char* policy) {
    if (strcmp(policy, "DROP") == 0) {
    return POLICY_DROP;
    } else if (strcmp(policy, "ACCEPT") == 0) {
    return POLICY_ACCEPT;
    }
    return POLICY_NONE;
}

int
parse_cmd_args(int argc, char *argv[],filter_rule_t* fr,std::string& file_name)
{
    int c; int command = CMD_PRINT_HELP;
    struct in_addr ipvalue;

    memset(fr,0,sizeof(filter_rule_t));

    bool proto_mandatory = false;

    while (1)
    {
        static const struct option long_options[] =
        {
            {"list", required_argument, NULL, 'L'},
            // {"new", required_argument, NULL, 'N'},
            {"append", required_argument, NULL, 'A'},
            {"delete", required_argument, NULL, 'D'},
            {"flush", required_argument, NULL, 'F'},
            {"update", required_argument, NULL, 'U'},
            {"policy", required_argument, NULL, 'P'},
            {"off", required_argument, NULL, 'o'},
            {"srcip", required_argument, NULL, 's'},
            {"source", required_argument, NULL, 's'},
            //{"srcnetmask", required_argument, NULL, 'm'},
            {"sport", required_argument, NULL, '1'},
            {"srcport", required_argument, NULL, '1'},
            {"source-port", required_argument, NULL, '1'},
            {"destip", required_argument, NULL, 'd'},
            {"destination", required_argument, NULL, 'd'},
            //{"destnetmask", required_argument, NULL, 'n'},
            {"dport", required_argument, NULL, '2'},
            {"destport", required_argument, NULL, '2'},
            {"destination-port", required_argument, NULL, '2'},
            {"proto", required_argument, NULL, 'p'},
            {"file", required_argument, NULL, 'f'},
            {"jump", required_argument, NULL, 'j'},
            {0, 0, 0, 0}
        };
        int option_index = 0;
        c = getopt_long(argc, argv, "L:A:D:F:U:P:s:1:d:2:p:o:f:j", long_options, &option_index);
        /*Detect the end of the options. */
        if (c == -1)
            break;

        switch (c)
        {
            case 0:
              //printf("flag option: %s, mf_rule.in_out = %d\n", long_options[option_index].name, mf_rule.in_out);
              break;
            case 'L':
                command = CMD_LIST;
                fr->base.chain = get_chain(optarg);
                if (fr->base.chain == CHAIN_NONE)
                {
                   std::cout << "Пропущен обязательный параметр название цепочки " << std::endl ;
                   command = CMD_PRINT_HELP;
                }
              break;
            case 'f':
                command = CMD_GET_FROM_FILE;
                if(optarg)
                    file_name = optarg;
                else
                    command = CMD_PRINT_HELP;

                break;
            case 'A':
                command = CMD_APPEND;
                fr->base.chain = get_chain(optarg);
                if (fr->base.chain == CHAIN_NONE)
                {
                   std::cout << "Пропущен обязательный параметр название цепочки " << std::endl ;
                   command = CMD_PRINT_HELP;
                }
                //printf("chain in_out = %d\n", fr->base.chain);
              break;
            case 'D':
              command = CMD_DELETE;       //delete
              fr->base.chain = get_chain(optarg);
              if (fr->base.chain == CHAIN_NONE)
              {
                 std::cout << "Пропущен обязательный параметр название цепочки " << std::endl ;
                 command = CMD_PRINT_HELP;
              }
              break;
            case 'F':
              command = CMD_FLUSH;       //delete
              fr->base.chain = get_chain(optarg);
              if (fr->base.chain == CHAIN_NONE)
              {
                 std::cout << "Пропущен обязательный параметр название цепочки " << std::endl ;
                 command = CMD_PRINT_HELP;
              }
              break;
            case 'U':
              command = CMD_UPDATE;       //delete
              fr->base.chain = get_chain(optarg);
              if (fr->base.chain == CHAIN_NONE)
              {
                 std::cout << "Пропущен обязательный параметр название цепочки " << std::endl ;
                 command = CMD_PRINT_HELP;
              }
              break;
            case 's':
             {
              int s = inet_pton(AF_INET, optarg, &ipvalue);
               switch(s) {
               case 1:
                 //printf("converted value = %x \n", ipvalue.s_addr);
                 fr->base.s_addr.addr = ipvalue.s_addr;
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
            case 'o':
              fr->off = get_switch(optarg); //switch
              if (fr->off==SW_NONE){
                std::cout << "Ошибка в параметре --off " << std::endl ;
                command = CMD_PRINT_HELP;
              }

              proto_mandatory = true;
              break;
            case '1':
              fr->base.src_port = atoi(optarg);    //srcport:
              break;
            case 'd':
              {
              //printf("converted value  d_addr.addr\n");
              int s = inet_pton(AF_INET, optarg, &ipvalue);
                   switch(s) {
                   case 1:
                  //printf("converted value = %x \n", ipvalue.s_addr);
                  fr->base.d_addr.addr = ipvalue.s_addr;
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
            case '2':
              fr->base.dst_port = atoi(optarg);    //destport
              break;
            case 'p':
              fr->base.proto = get_proto(optarg); //proto
              if (fr->base.proto==IPPROTO_NOTEXIST){
                std::cout << "Ошибка в параметре --proto " << std::endl ;
                command = CMD_PRINT_HELP;
              }

              proto_mandatory = true;
              break;
            case 'P':
               fr->policy = get_policy(optarg);
               command = CMD_SET_POLICY;
               if (fr->policy==POLICY_NONE){
                 std::cout << "Ошибка в параметре --policy " << std::endl ;
                 command = CMD_PRINT_HELP;
               }
              break;
            case 'j':
               fr->policy = get_policy(optarg);
               if (fr->policy==POLICY_NONE){
                 std::cout << "Ошибка в параметре --jump " << std::endl ;
                 command = CMD_PRINT_HELP;
               }
              break;
            case '?':
              /* getopt_long printed an error message. */
              break;
            default:
              abort();
        }

//        if (optind < argc) {
//            printf ("элементы ARGV, не параметры: ");
//            while (optind < argc)
//                printf ("%s ", argv[optind++]);
//            printf ("\n");
//        }


    }

    // proto обязательный параметр при добавлении удалении правил
   if(!proto_mandatory && (command == CMD_APPEND || command == CMD_DELETE ))
   {
       command = CMD_PRINT_HELP;
       std::cout << "- пропущен обязательный параметр --proto " << std::endl ;
   }
   return command;
}

}
