#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <netinet/ip.h> 
#include <linux/if_ether.h>
#include <netlink/netlink.h>
//#include <netlink/version.h>

#include "trx_data.h"

enum commands_t {
       CMD_NONE,
       CMD_INSERT,
       CMD_DELETE = 0x0002U,
       //CMD_DELETE_NUM = 0x0004U,
       //CMD_REPLACE=		0x0008U,
       CMD_APPEND=		0x0010U,
       CMD_LIST=		0x0020U,
       CMD_FLUSH=		0x0040U,
       //CMD_ZERO=		0x0080U,
       //CMD_NEW_CHAIN=		0x0100U,
       //CMD_DELETE_CHAIN=	0x0200U,
       CMD_SET_POLICY=		0x0400U,
       //CMD_RENAME_CHAIN=	0x0800U,
       //CMD_LIST_RULES=		0x1000U,
       //CMD_ZERO_NUM=		0x2000U,
       //CMD_CHECK=		0x4000U,
       CMD_UPDATE= 0x8000U,
       CMD_GET_FROM_FILE,
       CMD_PRINT_HELP
};

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

int
get_switch(char* off) {
    if (strcmp(off, "YES") == 0) {
        return SWITCH_YES;
    } else if (strcmp(off, "NO") == 0) {
        return SWITCH_NO;
    }

    return SWITCH_NONE;
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
}

int
parse_cmd_args(int argc, char *argv[],filter_rule_t* fr,const char* file_name)
{
    int c; int command = CMD_PRINT_HELP;
    struct in_addr ipvalue;

    memset(fr,0,sizeof(filter_rule_t));

    int proto_mandatory = 0;

    while (1)
    {
        static const struct option long_options[] =
        {
            {"list", required_argument, NULL, 'L'},
            // {"new", required_argument, NULL, 'N'},
            {"append", required_argument, NULL, 'A'},
            {"delete", required_argument, NULL, 'D'},
            {"flush", required_argument, NULL, 'F'},
            {"policy", required_argument, NULL, 'P'},
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
            {"off", required_argument, NULL, 'o'},
            {"file", required_argument, NULL, 'f'},
            {"jump", required_argument, NULL, 'j'},
            {0, 0, 0, 0}
        };
        int option_index = 0;
        c = getopt_long(argc, argv, "L:A:D:F:P:s:1:d:2:p:o:f:j", long_options, &option_index);
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
                   command = CMD_PRINT_HELP;
                //printf("chain in_out = %d\n", fr->base.chain);
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
                   command = CMD_PRINT_HELP;
                //printf("chain in_out = %d\n", fr->base.chain);
              break;
            case 'D':
              command = CMD_DELETE;       //delete
              fr->base.chain = get_chain(optarg);
              if (fr->base.chain == CHAIN_NONE)
                 command = CMD_PRINT_HELP;
              //printf("chain in_out = %d\n", fr->base.chain);
              break;
            case 'F':
              command = CMD_FLUSH;       //delete
              fr->base.chain = get_chain(optarg);
              if (fr->base.chain ==CHAIN_NONE)
                 command = CMD_PRINT_HELP;
              //printf("chain in_out = %d\n", fr->base.chain);
              break;
            case 's':
             {
              int s = inet_pton(AF_INET, optarg, &ipvalue);
               switch(s) {
               case 1:
                 // printf("converted value = %x \n", ipvalue.s_addr);
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
              if (fr->off==SWITCH_NONE){
                  printf("Ошибка в параметре --off \n");
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
                command = CMD_PRINT_HELP;
              }

              proto_mandatory = 1;
              break;
            case 'P':
               fr->policy = get_policy(optarg);
               command = CMD_SET_POLICY;
              break;
            case 'j':
               fr->policy = get_policy(optarg);
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
       printf ("- пропущен обязательный параметр --proto \n");
   }
   return command;
}

static const char * prog_name = "bf_ctrl";
static const char * prog_vers = "0.0.1a";

static void
exit_printhelp(/*const struct xtables_rule_match *matches*/)
{
	printf("%s v%s\n\n"
"Usage: %s -[ACD] chain rule-specification [options]\n"
"       %s -I chain [rulenum] rule-specification [options]\n"
"       %s -R chain rulenum rule-specification [options]\n"
"       %s -D chain rulenum [options]\n"
"       %s -[LS] [chain [rulenum]] [options]\n"
"       %s -[FZ] [chain] [options]\n"
"       %s -[NX] chain\n"
"       %s -E old-chain-name new-chain-name\n"
"       %s -P chain target [options]\n"
"       %s -h (print this help information)\n\n",
	       prog_name, prog_vers, prog_name, prog_name,
	       prog_name, prog_name, prog_name, prog_name,
	       prog_name, prog_name, prog_name, prog_name);

	printf(
"Commands:\n"
"Either long or short options are allowed.\n"
"  --append  -A chain		Append to chain\n"
"  --check   -C chain		Check for the existence of a rule\n"
"  --delete  -D chain		Delete matching rule from chain\n"
"  --delete  -D chain rulenum\n"
"				Delete rule rulenum (1 = first) from chain\n"
"  --insert  -I chain [rulenum]\n"
"				Insert in chain as rulenum (default 1=first)\n"
"  --replace -R chain rulenum\n"
"				Replace rule rulenum (1 = first) in chain\n"
"  --list    -L [chain [rulenum]]\n"
"				List the rules in a chain or all chains\n"
"  --list-rules -S [chain [rulenum]]\n"
"				Print the rules in a chain or all chains\n"
"  --flush   -F [chain]		Delete all rules in  chain or all chains\n"
"  --zero    -Z [chain [rulenum]]\n"
"				Zero counters in chain or all chains\n"
"  --new     -N chain		Create a new user-defined chain\n"
"  --delete-chain\n"
"            -X [chain]		Delete a user-defined chain\n"
"  --policy  -P chain target\n"
"				Change policy on chain to target\n"
"  --rename-chain\n"
"            -E old-chain new-chain\n"
"				Change chain name, (moving any references)\n"

"Options:\n"
"    --ipv4	-4		Nothing (line is ignored by ip6tables-restore)\n"
"    --ipv6	-6		Error (line is ignored by iptables-restore)\n"
"[!] --proto	-p proto	protocol: by number or name, eg. `tcp'\n"
"[!] --source	-s address[/mask][...]\n"
"				source specification\n"
"[!] --destination -d address[/mask][...]\n"
"				destination specification\n"
"[!] --in-interface -i input name[+]\n"
"				network interface name ([+] for wildcard)\n"
" --jump	-j target\n"
"				target for rule (may load target extension)\n"
#ifdef IPT_F_GOTO
"  --goto      -g chain\n"
"                              jump to chain with no return\n"
#endif
"  --match	-m match\n"
"				extended match (may load extension)\n"
"  --numeric	-n		numeric output of addresses and ports\n"
"[!] --out-interface -o output name[+]\n"
"				network interface name ([+] for wildcard)\n"
"  --table	-t table	table to manipulate (default: `filter')\n"
"  --verbose	-v		verbose mode\n"
"  --line-numbers		print line numbers when listing\n"
"  --exact	-x		expand numbers (display exact values)\n"
"[!] --fragment	-f		match second or further fragments only\n"
"  --modprobe=<command>		try to insert modules using this command\n"
"  --set-counters PKTS BYTES	set the counter during insert/append\n"
"[!] --version	-V		print package version.\n");

	// print_extension_helps(xtables_targets, matches);
	exit(0);
}

static int
nl_rcv_msg(struct nl_msg *msg, void *arg)
{
   printf("Goottooooooototototototo  callback \n");
}

__attribute__((constructor))
static void module_ctor(void)
{

}

__attribute__((destructor))
static void module_dtor(void) {

}


int
main(int argc, char *argv[])
{


#ifdef HAVE_LIBNL3
	struct nl_sock *nls;
#else
	struct nl_handle *nls;
#endif
    filter_rule_t fr;
    memset(&fr,0,sizeof(filter_rule_t));

// FIXME file name для сохраненных правил
    int action = parse_cmd_args(argc, argv,&fr,NULL);

    int ret=0;
#ifdef HAVE_LIBNL3
     nls = nl_socket_alloc();
#else
   //  nls = nl_handle_alloc();  
     struct nl_cb * cb = nl_cb_alloc(NL_CB_DEFAULT);
     nl_cb_set(cb, NL_CB_MSG_IN, NL_CB_DEFAULT,  nl_rcv_msg, NULL);
     nls = nl_handle_alloc_cb(cb);
#endif
   




    if (!nls) {
        printf("bad nl_socket_alloc\n");
        return EXIT_FAILURE;
    }

    ret = nl_connect(nls, NETLINK_USERSOCK);
    if (ret < 0) {
#ifdef HAVE_LIBNL3
	nl_perror(ret, "nl_connect");
        nl_socket_free(nls);
#else
	nl_perror("nl_connect");
	nl_handle_destroy(nls);
#endif
        return EXIT_FAILURE;
    } 
    if (action == CMD_APPEND) {
        printf("CMD_APPEND");
    printf("Src_addr: %X; dst_addr: %X; proto: %d; src_port: %d dst_port: %d\n", fr.base.s_addr.addr, fr.base.d_addr.addr, fr.base.proto, fr.base.src_port, fr.base.dst_port);


    	ret = nl_send_simple(nls, MSG_ADD_RULE, 0, &fr, sizeof(fr));

    	if (ret < 0) {
#ifdef HAVE_LIBNL3
        	nl_perror(ret, "nl_send_simple");
        	nl_close(nls);
        	nl_socket_free(nls);
#else
        	nl_perror("nl_send_simple");
        	nl_close(nls);
		nl_handle_destroy(nls);
#endif
        	return EXIT_FAILURE;
    	} else {
        	//printf("sent %d bytes\n", ret);
		printf(".");		
		//total_cb += ret;
    	}


    } else if (action == CMD_LIST) {
        printf("CMD_LIST\n");
	// printf("pid %d:\n",getpid());



    	ret = nl_send_simple(nls, MSG_GET_RULES, 0, &fr, sizeof(fr));

    	if (ret < 0) {
#ifdef HAVE_LIBNL3
        	nl_perror(ret, "nl_send_simple");
        	nl_close(nls);
        	nl_socket_free(nls);
#else
        	nl_perror("nl_send_simple");
        	nl_close(nls);
		nl_handle_destroy(nls);
#endif
        	return EXIT_FAILURE;
    	} else {
        	//printf("sent %d bytes\n", ret);
		//printf(".");		
		//total_cb += ret;
    	}

	
	struct sockaddr_nl peer;
 	unsigned char *nl_msg;
	unsigned char *msg;
	struct ucred *creds;

	int msgn,n;
	struct nlmsghdr *hdr;
	int nlerrr =0;
    int c = 0;
        printf("List of rules:\n");
	do{ 
		//nl_recvmsgs_default(nls);
		msgn = n = nl_recv(nls, NULL, &nl_msg,&creds);
		hdr = (struct nlmsghdr *) nl_msg;

		if(n<0) printf("nl_recv err= %d\n", n);

	while (nlmsg_ok(hdr, n)) {
         msg = NLMSG_DATA((struct nlmsghdr *)hdr);
		 //printf("hdr->nlmsg_type: %d hdr->nlmsg_flags: %d \n",hdr->nlmsg_type,hdr->nlmsg_flags);
		
        if(hdr->nlmsg_type==NLMSG_ERROR){
			struct nlmsgerr *nlerr;
			nlerr = (struct nlmsgerr*)NLMSG_DATA((struct nlmsghdr *)nl_msg);
			if(nlerrr!=0) printf("Got some error: %d \n",nlerr->error);
			nlerrr = nlerr->error;
		}

        if(hdr->nlmsg_type==NLMSG_ERROR || hdr->nlmsg_type==MSG_DONE) break;
		
		 // 
		// printf("hdr->nlmsg_type: %d nlmsg_len: %d nlmsg_flags : %d nlmsg_seq : %d nlmsg_pid : %d \n",hdr->nlmsg_type,hdr->nlmsg_len,hdr->nlmsg_flags,hdr->nlmsg_seq,hdr->nlmsg_pid);
		
		if(hdr->nlmsg_type==MSG_DATA)
	        printf("# %d src port: %d  dst port: %d d_addr: %d s_addr: %d proto: %s (%d)\n",
            /*((filter_rule_t*)msg)->id*/++c,((filter_rule_t*)msg)->base.src_port,
            ((filter_rule_t*)msg)->base.dst_port,((filter_rule_t*)msg)->base.d_addr.addr,
            ((filter_rule_t*)msg)->base.s_addr.addr,get_proto_name(((filter_rule_t*)msg)->base.proto)==NULL?"":get_proto_name(((filter_rule_t*)msg)->base.proto),
            ((filter_rule_t*)msg)->base.proto);

		if(hdr->nlmsg_flags&NLM_F_ACK){
		    filter_rule_t msg;	
		    memset(&msg,0,sizeof(filter_rule_t));  
		    ret = nl_send_simple(nls, MSG_OK/*NLMSG_ERROR*/, 0, &msg, sizeof(filter_rule_t));
		    // printf("Got the flag NLM_F_ACK, ret =%d\n",ret);
		    if (ret < 0) {
		#ifdef HAVE_LIBNL3
			nl_perror(ret, "nl_send_simple");
			nl_close(nls);
			nl_socket_free(nls);
		#else
			nl_perror( "nl_send_simple");
			nl_close(nls);	
			nl_handle_destroy(nls);
		#endif
			printf("Cant sent MSG_OK err: %d \n",ret);
		       	return EXIT_FAILURE;
		    }
        }

        hdr = nlmsg_next(hdr, &n);
	}

	}while(/*hdr->nlmsg_type!=NLMSG_ERROR*/ nlerrr==0 && hdr->nlmsg_type!=MSG_DONE && msgn>0);



    } else if (action == CMD_DELETE) {
        printf("CMD_DELETE\n");

    	ret = nl_send_simple(nls, MSG_DELETE_RULE, 0, &fr, sizeof(fr));

    	if (ret < 0) {
#ifdef HAVE_LIBNL3
        	nl_perror(ret, "nl_send_simple");
        	nl_close(nls);
        	nl_socket_free(nls);
#else
        	nl_perror("nl_send_simple");
        	nl_close(nls);
		nl_handle_destroy(nls);
#endif
        	return EXIT_FAILURE;
    	} else {
        	//printf("sent %d bytes\n", ret);
		//printf(".");		
		//total_cb += ret;
    	}
    } else if (action == CMD_FLUSH) {
        printf("CMD_FLUSH\n");

    	ret = nl_send_simple(nls, MSG_DELETE_ALL_RULES, 0, &fr, sizeof(fr));

    	if (ret < 0) {
#ifdef HAVE_LIBNL3
        	nl_perror(ret, "nl_send_simple");
        	nl_close(nls);
        	nl_socket_free(nls);
#else
        	nl_perror("nl_send_simple");
        	nl_close(nls);
		nl_handle_destroy(nls);
#endif
        	return EXIT_FAILURE;
    	} else {
        	//printf("sent %d bytes\n", ret);
		//printf(".");		
		//total_cb += ret;
    	}
    } else if (action == CMD_UPDATE) {
        // qDebug() << "CMD_UPDATE do not realized\n";
        printf("CMD_UPDATE do not realized\n");
        // bfc->updateRule(fr);
    } else if (action == CMD_PRINT_HELP) {
	exit_printhelp();
    } 

    filter_rule_t msg;	
    memset(&msg,0,sizeof(filter_rule_t));  
    ret = nl_send_simple(nls, MSG_DONE, 0, &msg, sizeof(filter_rule_t));

    if (ret < 0) {
#ifdef HAVE_LIBNL3
        nl_perror(ret, "nl_send_simple");
        nl_close(nls);
        nl_socket_free(nls);
#else
        nl_perror( "nl_send_simple");
        nl_close(nls);
	nl_handle_destroy(nls);
#endif
       	return EXIT_FAILURE;
    }
   //  printf("\nTotal sent %d bytes\n", total_cb);
    
    nl_close(nls);
#ifdef HAVE_LIBNL3
    nl_socket_free(nls);
#else
    nl_handle_destroy(nls);
#endif

    return EXIT_SUCCESS;
}
