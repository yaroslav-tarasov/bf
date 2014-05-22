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

static const char * prog_name = "bf_ctrl";
static const char * prog_vers = "0.0.1a";

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
parse_cmd_args(int argc, char *argv[], cmd_args& ca )
{
    int c; int command = CMD_PRINT_HELP;
    struct in_addr ipvalue;
    filter_rule_t* fr = &ca.fr;
    std::string& file_name = ca.file_name;

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
                   std::cerr << "Пропущен обязательный параметр название цепочки " << std::endl ;
                   command = CMD_PRINT_HELP;
                }


              break;
            case 'f':
                command = CMD_LOAD_FROM_FILE;
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
                   std::cerr << "Пропущен обязательный параметр название цепочки " << std::endl ;
                   command = CMD_PRINT_HELP;
                }
                //printf("chain in_out = %d\n", fr->base.chain);
              break;
            case 'D':
              command = CMD_DELETE;       //delete
              fr->base.chain = get_chain(optarg);
              if (fr->base.chain == CHAIN_NONE)
              {
                 std::cerr << "Пропущен обязательный параметр название цепочки " << std::endl ;
                 command = CMD_PRINT_HELP;
              }
              break;
            case 'F':
              command = CMD_FLUSH;       //delete
              fr->base.chain = get_chain(optarg);
              if (fr->base.chain == CHAIN_NONE)
              {
                 std::cerr << "Пропущен обязательный параметр название цепочки " << std::endl ;
                 command = CMD_PRINT_HELP;
              }
              break;
            case 'U':
              command = CMD_UPDATE;       //delete
              fr->base.chain = get_chain(optarg);
              if (fr->base.chain == CHAIN_NONE)
              {
                 std::cerr << "Пропущен обязательный параметр название цепочки " << std::endl ;
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
            //case 'm':
              //mf_rule.src_netmask = optarg; //srcnetmask:
            //  break;
            case 'o':
              fr->off = get_switch(optarg); //switch
              if (fr->off==SW_NONE){
                std::cerr << "Ошибка в параметре --off " << std::endl ;
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
                std::cerr << "Ошибка в параметре --proto " << std::endl ;
                command = CMD_PRINT_HELP;
              }

              proto_mandatory = true;
              break;
            case 'P':
               fr->base.chain = get_chain(optarg);
               if (fr->base.chain == CHAIN_NONE)
               {
                   std::cerr << "Пропущен обязательный параметр название цепочки " << std::endl ;
                   command = CMD_PRINT_HELP;
                   break;
               }
               if (optind < argc && argv[optind][0] != '-'
                   && argv[optind][0] != '!'){
                   fr->policy = get_policy(argv[optind++]);
                   command = CMD_SET_POLICY;
                   if (fr->policy==POLICY_NONE){
                       std::cerr << "Ошибка в параметре policy(ACCEPT,DROP) " << std::endl ;
                     command = CMD_PRINT_HELP;
                   }
               }
               else
                   std::cerr << "Требуются и параметр chain(INPUT,OUTPUT) и параметр policy (ACCEPT,DROP)" << std::endl ;

              break;
            case 'j':
               fr->policy = get_policy(optarg);
               if (fr->policy==POLICY_NONE){
                 std::cerr << "Ошибка в параметре --jump " << std::endl ;
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
       std::cerr << "- пропущен обязательный параметр --proto " << std::endl ;
   }
   return command;
}

void exit_printhelp()
{
    printf("%s v%s\n\n"
"Usage: %s -[AD] chain rule-specification [options]\n"
//"       %s -I chain [rulenum] rule-specification [options]\n"
//"       %s -R chain rulenum rule-specification [options]\n"
"       %s -U chain  rule-specification [options]\n"
//"       %s -D chain rulenum [options]\n"
"       %s -L [chain [rulenum]] [options]\n"
"       %s -F [chain] [options]\n"
// "       %s -[NX] chain\n"
// "       %s -E old-chain-name new-chain-name\n"
"       %s -P chain target [options]\n"
"       %s -h (print this help information)\n\n",
           prog_name, prog_vers, prog_name, prog_name,
           prog_name, prog_name, prog_name , prog_name/*,
           prog_name,prog_name, prog_name, prog_name*/);

    printf(
"Commands:\n"
"Either long or short options are allowed.\n"
"  --append  -A chain		Append to chain\n"
//"  --check   -C chain		Check for the existence of a rule\n"
"  --delete  -D chain		Delete matching rule from chain\n"
//"  --delete  -D chain rulenum\n"
//"				Delete rule rulenum (1 = first) from chain\n"
"  --update  -U chain		Update matching rule target or/and turn rule on/off\n"

//"  --insert  -I chain [rulenum]\n"
//"				Insert in chain as rulenum (default 1=first)\n"
//"  --replace -R chain rulenum\n"
//"				Replace rule rulenum (1 = first) in chain\n"
"  --list    -L [chain [rulenum]]\n"
"				List the rules in a chain or all chains\n"
//"  --list-rules -S [chain [rulenum]]\n"
//"				Print the rules in a chain or all chains\n"
"  --flush   -F [chain]		Delete all rules in  chain or all chains\n"
//"  --zero    -Z [chain [rulenum]]\n"
//"				Zero counters in chain or all chains\n"
//"  --new     -N chain		Create a new user-defined chain\n"
//"  --delete-chain\n"
//"            -X [chain]		Delete a user-defined chain\n"
"  --policy  -P chain target\n"
"				Change policy on chain to target\n"
//"  --rename-chain\n"
//"            -E old-chain new-chain\n"
//"				Change chain name, (moving any references)\n"

"Options:\n"
//"    --ipv4	-4		Nothing (line is ignored by ip6tables-restore)\n"
//"    --ipv6	-6		Error (line is ignored by iptables-restore)\n"
"[!] --proto	-p proto	protocol: by number or name, eg. `tcp'\n"
"[!] --source	-s address[/mask][...]\n"
"				source specification\n"
"[!] --destination -d address[/mask][...]\n"
"				destination specification\n"
//"[!] --in-interface -i input name[+]\n"
//"				network interface name ([+] for wildcard)\n"
" --jump	-j target\n"
"				target for rule (may load target extension)\n"
#ifdef IPT_F_GOTO
"  --goto      -g chain\n"
"                              jump to chain with no return\n"
#endif
//"  --match	-m match\n"
//"				extended match (may load extension)\n"
//"  --numeric	-n		numeric output of addresses and ports\n"
//"[!] --out-interface -o output name[+]\n"
//"				network interface name ([+] for wildcard)\n"
//"  --table	-t table	table to manipulate (default: `filter')\n"
//"  --verbose	-v		verbose mode\n"
//"  --line-numbers		print line numbers when listing\n"
"//  --exact	-x		expand numbers (display exact values)\n"
//"[!] --fragment	-f		match second or further fragments only\n"
//"  --modprobe=<command>		try to insert modules using this command\n"
//"  --set-counters PKTS BYTES	set the counter during insert/append\n"
"[!] --version	-V		print package version.\n");

    // print_extension_helps(xtables_targets, matches);
    // exit(0);
}

}
