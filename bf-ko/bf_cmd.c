#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <netinet/ip.h> 
#include <linux/if_ether.h>
#include <netlink/netlink.h>
//#include <netlink/version.h>

#include "trx_data.h"

enum { CMD_NEW_RULE=1,CMD_PRINT_RULES,CMD_DEL_RULE,CMD_PRINT_HELP};
enum { DIR_ALL,DIR_INPUT,DIR_OUTPUT};

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

    int action = parse_cmd_args(argc, argv,&fr);

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
    if (action == CMD_NEW_RULE) {
        printf("CMD_NEW_RULE\n");
	printf("Src_addr: %X; dst_addr: %X; proto: %d; src_port: %d dst_port: %d\n", fr.base_rule.s_addr.addr, fr.base_rule.d_addr.addr, fr.base_rule.proto, fr.base_rule.src_port, fr.base_rule.dst_port);


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


    } else if (action == CMD_PRINT_RULES) {
        printf("CMD_PRINT_RULES\n");
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
// 	int ret = nl_recv(nls, &peer, &nl_msg,&creds);

	int msgn,n;
	//unsigned char *buf; 
	struct nlmsghdr *hdr;
	int nlerrr =0;

        printf("List of rules:\n");
	do{ 
		//nl_recvmsgs_default(nls);
		msgn = n = nl_recv(nls, NULL, &nl_msg,&creds);
		hdr = (struct nlmsghdr *) nl_msg;
		msg = NLMSG_DATA((struct nlmsghdr *)nl_msg);
		//printf(" %s nl_recv return %d %d\n", __func__,msgn,n);

		if(n<0) printf("nl_recv err= %d\n", n);

	while (nlmsg_ok(hdr, n)) {
	         msg = NLMSG_DATA((struct nlmsghdr *)nl_msg);
		 //printf("hdr->nlmsg_type: %d hdr->nlmsg_flags: %d \n",hdr->nlmsg_type,hdr->nlmsg_flags);
		
		if(hdr->nlmsg_type==NLMSG_ERROR)
		{	
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
			((filter_rule_t*)msg)->id,((filter_rule_t*)msg)->base_rule.src_port,
			((filter_rule_t*)msg)->base_rule.dst_port,((filter_rule_t*)msg)->base_rule.d_addr.addr,
			((filter_rule_t*)msg)->base_rule.s_addr.addr,get_proto_name(((filter_rule_t*)msg)->base_rule.proto)==NULL?"":get_proto_name(((filter_rule_t*)msg)->base_rule.proto),
			((filter_rule_t*)msg)->base_rule.proto);

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

	/*msgn = n = nl_recv(nls, NULL, &nl_msg,&creds);
	hdr = (struct nlmsghdr *) nl_msg;
	msg = NLMSG_DATA((struct nlmsghdr *)nl_msg);
	printf("hdr->nlmsg_type: %d nlmsg_len: %d nlmsg_flags : %d nlmsg_seq : %d nlmsg_pid : %d \n",hdr->nlmsg_type,hdr->nlmsg_len,hdr->nlmsg_flags,hdr->nlmsg_seq,hdr->nlmsg_pid);
        printf("# %d src port: %d  dst port: %d d_addr: %d s_addr: %d proto: %d\n",
		((filter_rule_t*)msg)->id,((filter_rule_t*)msg)->base_rule.src_port,
		((filter_rule_t*)msg)->base_rule.dst_port,((filter_rule_t*)msg)->base_rule.d_addr.addr,
		((filter_rule_t*)msg)->base_rule.s_addr.addr,((filter_rule_t*)msg)->base_rule.proto);

	if(hdr->nlmsg_type==NLMSG_ERROR)
	{	
		struct nlmsgerr *nlerr;
		nlerr = (struct nlmsgerr*)NLMSG_DATA((struct nlmsghdr *)nl_msg);
		printf("Got some error: %d \n",nlerr->error);
	}*/

	


    } else if (action == CMD_DEL_RULE) {
        printf("CMD_DEL_RULE\n");

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
		printf(".");		
		//total_cb += ret;
    	}
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
