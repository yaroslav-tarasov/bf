#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/netfilter.h>
#undef __KERNEL__
#include <linux/netfilter_ipv4.h>
#define __KERNEL__
#include <linux/netfilter_bridge.h>
#include <linux/skbuff.h>
#include <linux/udp.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <linux/tcp.h>
#include <linux/proc_fs.h>  /* Necessary because we use the proc fs */
#include <linux/list.h>
#include <net/ip.h>
#include <asm/uaccess.h>	/* For copy_from_user  */
#include "base/hash_table.h"
#include "nl_int.h"
#include "trx_data.h"


struct nf_hook_ops nfho_in;   //net filter hook option struct
struct nf_hook_ops nfho_out;  //net filter hook option struct

DEFINE_SPINLOCK(list_mutex);	

#define skb_filter_name "bf_filter"

int nl_send_msg(struct sock * nl_sk,int destpid, int type,int flags,char* msg,int msg_size);
void wfc(void);

typedef struct filter_rule_list {
    
    filter_rule_t fr;
    struct list_head full_list; /* kernel's list structure */
    struct list_head protocol_list; /* kernel's list structure */
    struct hash_entry entry;
} filter_rule_list_t;

 
static struct filter_rule_list lst_fr;
static struct filter_rule_list lst_fr_udp;
static struct filter_rule_list lst_fr_tcp;
struct hash_table map_fr;

static  void init_rules(void)
{    
    struct filter_rule_list *a_new_fr, *a_rule; 
    int i;  
    
    
    hash_table_init(&map_fr, 10, NULL);

/* adding elements to mylist */
#if 0
    uint8_t rb;
///   This code using just for testing purpose
    for(i=0; i<20000; ++i){
	
	get_random_bytes ( &rb, sizeof (uint8_t) );
        a_new_fr = kmalloc(sizeof(*a_new_fr), GFP_KERNEL);
        a_new_fr->fr.base_rule.d_addr.addr = 0;
        a_new_fr->fr.base_rule.s_addr.addr = 0;
        a_new_fr->fr.base_rule.proto = IPPROTO_UDP;// rb<128?IPPROTO_UDP:IPPROTO_TCP;
        a_new_fr->fr.base_rule.src_port = 53 + i;
	a_new_fr->fr.base_rule.dst_port = 53 + i;
	a_new_fr->fr.off = 0;
        //INIT_LIST_HEAD(&a_new_fr->full_list);
        // add the new node to mylist 
        list_add(&(a_new_fr->full_list), &(lst_fr.full_list));//list_add_tail(&(a_new_fr->list), &(lst_fr.list));
	hash_table_insert(&map_fr, &a_new_fr->entry, (const char*)&a_new_fr->fr.base_rule, sizeof(struct filter_rule_base));
	
	if(a_new_fr->fr.base_rule.proto == IPPROTO_UDP)
		list_add(&(a_new_fr->protocol_list), &(lst_fr_udp.protocol_list));
	else if (a_new_fr->fr.base_rule.proto == IPPROTO_TCP)
		list_add(&(a_new_fr->protocol_list), &(lst_fr_tcp.protocol_list));		
	
    }
#else
	a_new_fr = kmalloc(sizeof(*a_new_fr), GFP_KERNEL);
        a_new_fr->fr.base_rule.d_addr.addr = 0;
        a_new_fr->fr.base_rule.s_addr.addr = 0;
        a_new_fr->fr.base_rule.proto = IPPROTO_UDP;
        a_new_fr->fr.base_rule.src_port = 0;
	a_new_fr->fr.base_rule.dst_port = 0;
	a_new_fr->fr.policy = POLICY_ACCEPT;
	a_new_fr->fr.off = 0;
        //INIT_LIST_HEAD(&a_new_fr->full_list);
        // add the new node to mylist 
        list_add(&(a_new_fr->full_list), &(lst_fr.full_list));//list_add_tail(&(a_new_fr->list), &(lst_fr.list));
	hash_table_insert(&map_fr, &a_new_fr->entry, (const char*)&a_new_fr->fr.base_rule, sizeof(struct filter_rule_base));
	
	if(a_new_fr->fr.base_rule.proto == IPPROTO_UDP)
		list_add(&(a_new_fr->protocol_list), &(lst_fr_udp.protocol_list));
	
	a_new_fr = kmalloc(sizeof(*a_new_fr), GFP_KERNEL);
        a_new_fr->fr.base_rule.d_addr.addr = 0;
        a_new_fr->fr.base_rule.s_addr.addr = 0;
        a_new_fr->fr.base_rule.proto = IPPROTO_TCP;
        a_new_fr->fr.base_rule.src_port = 0;
	a_new_fr->fr.base_rule.dst_port = 0;
	a_new_fr->fr.off = 0;	
	a_new_fr->fr.policy = POLICY_ACCEPT;
	
	if (a_new_fr->fr.base_rule.proto == IPPROTO_TCP)
		list_add(&(a_new_fr->protocol_list), &(lst_fr_tcp.protocol_list));	
#endif
     
    i =0;
    list_for_each_entry(a_rule, &lst_fr.full_list, full_list) {
        printk(KERN_INFO "#%d Src_addr: %X; dst_addr: %X; proto: %d; src_port: %d dst_port: %d\n", i++,a_rule->fr.base_rule.s_addr.addr, a_rule->fr.base_rule.d_addr.addr, a_rule->fr.base_rule.proto, a_rule->fr.base_rule.src_port, a_rule->fr.base_rule.dst_port);
    
     }

}

static void del_rules(void)
{ 
    struct filter_rule_list *a_rule, *tmp;
        
    list_for_each_entry_safe(a_rule, tmp, &lst_fr_udp.protocol_list, protocol_list){
         list_del(&a_rule->protocol_list);
    }

    list_for_each_entry_safe(a_rule, tmp, &lst_fr_tcp.protocol_list, protocol_list){
         list_del(&a_rule->protocol_list);
    }

    hash_table_finit(&map_fr);

    list_for_each_entry_safe(a_rule, tmp, &lst_fr.full_list, full_list){
         // printk(KERN_INFO "freeing node %s\n", a_rule->name);
         list_del(&a_rule->full_list);
         kfree(a_rule);
    }
}

void add_rule(struct filter_rule* fr)
{
	struct filter_rule_list *a_new_fr;
	a_new_fr = kmalloc(sizeof(*a_new_fr), GFP_KERNEL);	
	memcpy(&a_new_fr->fr,fr,sizeof(filter_rule_t));

spin_lock(&list_mutex);
        list_add(&(a_new_fr->full_list), &(lst_fr.full_list));//list_add_tail(&(a_new_fr->list), &(lst_fr.list));
	hash_table_insert(&map_fr, &a_new_fr->entry, (const char*)&a_new_fr->fr.base_rule, sizeof(struct filter_rule_base));
	if(a_new_fr->fr.base_rule.proto == IPPROTO_UDP)
		list_add(&(a_new_fr->protocol_list), &(lst_fr_udp.protocol_list));
	else if (a_new_fr->fr.base_rule.proto == IPPROTO_TCP)
		list_add(&(a_new_fr->protocol_list), &(lst_fr_tcp.protocol_list));
spin_unlock(&list_mutex);	
	
}

inline int cmp_rule(struct filter_rule* fr1,struct filter_rule* fr2)
{
	return memcmp(fr1,fr2,sizeof(struct filter_rule));
}

void delete_rule(struct filter_rule* fr)
{
	struct filter_rule_list *a_rule, *tmp;

//spin_lock(&list_mutex);
	list_for_each_entry_safe(a_rule, tmp, &lst_fr_udp.protocol_list, protocol_list){
		if(cmp_rule(&a_rule->fr,fr)==0)
		{		
			list_del(&a_rule->protocol_list);
			break;
		}
	}

	list_for_each_entry_safe(a_rule, tmp, &lst_fr_tcp.protocol_list, protocol_list){
		if(cmp_rule(&a_rule->fr,fr)==0)
		{		
			list_del(&a_rule->protocol_list);
			break;
		}
	}

	hash_table_del_key_safe(&map_fr,(const char*)&fr->base_rule, sizeof(struct filter_rule_base));
	
	list_for_each_entry_safe(a_rule, tmp, &lst_fr.full_list, full_list){
		if(cmp_rule(&a_rule->fr,fr)==0){		
			list_del(&a_rule->full_list);
			kfree(a_rule);
		}
	}
//spin_unlock(&list_mutex);	
	
}

void list_rules(struct sock * nl_sk,int destpid)
{
    struct filter_rule_list *a_rule; 
    int i=0,ret; 
    int flags = 0;
    list_for_each_entry(a_rule, &lst_fr.full_list, full_list) {

        if(++i%220==0) { flags = NLM_F_ACK; };
	
	printk(KERN_INFO "#%d Src_addr: %X; dst_addr: %X; proto: %d; src_port: %d dst_port: %d\n", i,
			a_rule->fr.base_rule.s_addr.addr, a_rule->fr.base_rule.d_addr.addr, 
			a_rule->fr.base_rule.proto, a_rule->fr.base_rule.src_port, a_rule->fr.base_rule.dst_port);

	a_rule->fr.id = i; 	
	ret=nl_send_msg(nl_sk,destpid, MSG_DATA, flags,(char*)&a_rule->fr,sizeof(a_rule->fr));
	if(ret<0)
		 return;
	
	if(flags == NLM_F_ACK) {
	 	flags = 0; wfc();
	}
    }
    
    nl_send_msg(nl_sk,destpid, MSG_DONE, 0, (char*)&a_rule->fr,sizeof(a_rule->fr));
}

int find_rule(unsigned char* data)
{
    struct hash_entry *hentry;
	if ((hentry =
	     hash_table_lookup_key(&map_fr, data,
				   sizeof(filter_rule_base_t))) == NULL) {
	return -1;
	} else {
		/* just like the listr_item() */
		struct filter_rule_list *tmp;
		tmp = hash_entry(hentry, struct filter_rule_list, entry);
	return 0;	
	}
}



static struct proc_dir_entry *skb_filter;
 
static int filter_value = 1;
 
unsigned int hook_func(unsigned int hooknum, 
            struct sk_buff *skb, 
            const struct net_device *in, 
            const struct net_device *out, 
            int (*okfn)(struct sk_buff *))
{	
    struct sk_buff *sock_buff;
    struct udphdr *udp_header;      // UDP header struct
    struct iphdr *ip_header;        // IP header struct
    struct icmphdr *icmp_header;	// ICMP Header
    struct tcphdr *tcp_header;	// TCP Header
    struct ethhdr  *ethheader;      // Ethernet Header

    sock_buff = skb;	
 
    ethheader = (struct ethhdr*) skb_mac_header(sock_buff); 
    ip_header = (struct iphdr *) skb_network_header(sock_buff);
 
    if(!sock_buff || !ip_header || !ethheader || filter_value==0)
        return NF_ACCEPT;



    if(ip_header->protocol == IPPROTO_UDP){
        udp_header = (struct udphdr *)(skb_transport_header(sock_buff) + ip_hdrlen(sock_buff));
        if(udp_header){
	    struct filter_rule_list  *a_rule;
//spin_lock(&list_mutex);   
#if 1
	   list_for_each_entry(a_rule, &lst_fr_udp.protocol_list, protocol_list) {
		if((ntohs(udp_header->source) == a_rule->fr.base_rule.src_port || ntohs(udp_header->dest) == a_rule->fr.base_rule.dst_port) &&
		!a_rule->fr.off){
			printk(KERN_INFO "TID %d SRC: (%u.%u.%u.%u):%d --> DST: (%u.%u.%u.%u):%d proto: %d; \n", 
				(int)current->pid, NIPQUAD(ip_header->saddr),ntohs(udp_header->source),
				NIPQUAD(ip_header->daddr),ntohs(udp_header->dest), a_rule->fr.base_rule.proto);
			return NF_DROP;
		}
	    }
#endif
//spin_unlock(&list_mutex);

        }else
            return NF_DROP;
    } else  if(ip_header->protocol == IPPROTO_TCP){
        //printk(KERN_INFO "---------- TCP -------------\n");
        tcp_header = (struct tcphdr *)(skb_transport_header(sock_buff) + ip_hdrlen(sock_buff));
        if(tcp_header){	
	    struct filter_rule_list  *a_rule;
//spin_lock(&list_mutex);   
#if 1
	   list_for_each_entry(a_rule, &lst_fr_tcp.protocol_list, protocol_list) {
		if((ntohs(tcp_header->source) == a_rule->fr.base_rule.src_port || ntohs(tcp_header->dest) == a_rule->fr.base_rule.dst_port) &&
		!a_rule->fr.off){
			printk(KERN_INFO "TID %d SRC: (%u.%u.%u.%u):%d --> DST: (%u.%u.%u.%u):%d proto: %d; \n", 
				(int)current->pid, NIPQUAD(ip_header->saddr),ntohs(tcp_header->source),
				NIPQUAD(ip_header->daddr),ntohs(tcp_header->dest), a_rule->fr.base_rule.proto);
			return NF_DROP;
		}
	    }
#endif
//spin_unlock(&list_mutex);
	
            //printk(KERN_INFO "SRC: (%u.%u.%u.%u) --> DST: (%u.%u.%u.%u)\n",NIPQUAD(ip_header->saddr),NIPQUAD(ip_header->daddr));
            //printk(KERN_INFO "ICMP type: %d - ICMP code: %d\n",icmp_header->type, icmp_header->code);
        }else
            return NF_DROP;	
    } else  if(ip_header->protocol == IPPROTO_ICMP){
        //printk(KERN_INFO "---------- ICMP -------------\n");
        icmp_header = (struct icmphdr *)(skb_transport_header(sock_buff) + ip_hdrlen(sock_buff));
        if(icmp_header){		
	    // printk(KERN_INFO "SRC: (%pM) --> DST: (%pM)\n",ethheader->h_source,ethheader->h_dest);

            if(ethheader && skb_mac_header_was_set(skb)) printk(KERN_INFO "SRC: (%pM) --> DST: (%pM)\n",ethheader->h_source,ethheader->h_dest); 

            //printk(KERN_INFO "SRC: (%u.%u.%u.%u) --> DST: (%u.%u.%u.%u)\n",NIPQUAD(ip_header->saddr),NIPQUAD(ip_header->daddr));
            //printk(KERN_INFO "ICMP type: %d - ICMP code: %d  in %s  out %s \n",icmp_header->type, icmp_header->code,in!=NULL?"true":"false",out!=NULL?"true":"false");
        }else
            return NF_DROP;	
    }
 
    return filter_value != 0 ? NF_ACCEPT : NF_DROP;
}
 
int skb_read(char *page, char **start, off_t off,
            int count, int *eof, void *data)
{	
    int len;
 
    if(off > 0){
        *eof = 1;
        return 0;
    }
 
    if(count < sizeof(int)){
        *eof = 1;
        return -ENOSPC;
    }
 
    /* cpy to userspace */
    memcpy(page, &filter_value, sizeof(int));
    len = sizeof(int);
 
    return len;
}
 
int skb_write(struct file *file, const char *buffer, unsigned long len,
            void *data)
{
    unsigned char userData;
 
    if(len > PAGE_SIZE || len < 0){
        printk(KERN_INFO "Barrier Mini-Firewall: cannot allow space for data\n");
        return -ENOSPC;
    }
 
    /* write data to the buffer */
    if(copy_from_user(&userData, buffer, 1)){
        printk(KERN_INFO "Barrier Mini-Firewall: cannot copy data from userspace. OH NOES\n");
        return -EFAULT;
    }
 
    filter_value = simple_strtol(&userData, NULL, 10);
 
    return len;
} 
 
int init_module()
{   

    struct proc_dir_entry proc_root;
    int ret = 0;
    // LIST_HEAD(lst_fr);  // This macro leads to kernel panic on  list_add
    INIT_LIST_HEAD(&lst_fr.full_list);	
    INIT_LIST_HEAD(&lst_fr_udp.protocol_list);	
    INIT_LIST_HEAD(&lst_fr_tcp.protocol_list);

    // mutex_init(&list_mutex);

    init_rules();
	
    skb_filter = create_proc_entry( skb_filter_name, 0644, NULL);
 
    // If we cannot create the proc entry
    if(skb_filter == NULL){
        ret = -ENOMEM;
        if( skb_filter )
            remove_proc_entry( skb_filter_name, &proc_root);
 
        printk(KERN_INFO "Barrier Mini-Firewall: Could not allocate memory.\n");
        goto error;
 
    }else{		
        skb_filter->read_proc = skb_read;
        skb_filter->write_proc = skb_write;
        //skb_filter->owner = THIS_MODULE;	
    }	
 
    // Netfilter hook information, specify where and when we get the SKB
    nfho_out.hook = hook_func;
    // nfho.hooknum = NF_INET_PRE_ROUTING;
    nfho_out.hooknum = NF_INET_POST_ROUTING;

    nfho_out.pf = PF_INET;
    nfho_out.priority = NF_IP_PRI_LAST;
    //nfho_out.priority = NF_IP_PRI_FIRST;
#if (LINUX_VERSION_CODE >= 0x020500)     nfho_out.owner = THIS_MODULE;
#endif

    // nf_register_hook(&nfho_out);
 
    nfho_in.hook = hook_func;
    nfho_in.hooknum = NF_INET_PRE_ROUTING;
    // nfho.hooknum = NF_INET_POST_ROUTING;

    nfho_in.pf = PF_INET;
    //nfho.priority = NF_IP_PRI_LAST;
    nfho_in.priority = NF_IP_PRI_FIRST;
#if (LINUX_VERSION_CODE >= 0x020500)     nfho_in.owner = THIS_MODULE;
#endif
    nf_register_hook(&nfho_in);

    printk(KERN_INFO "Registering Barrier Mini-Firewall module\n");
    //
    nl_init();

error:
    return ret;
}
 
void cleanup_module()
{
    nf_unregister_hook(&nfho_in);
    //nf_unregister_hook(&nfho_out);

    if ( skb_filter )
        remove_proc_entry(skb_filter_name, NULL);
    
    nl_exit();
    
    del_rules();
    
    //mutex_destroy(&list_mutex);
    
    printk(KERN_INFO "Unregistered the Barrier Mini-Firewall module\n");
}
 
MODULE_AUTHOR("Yaroslav Tarasov");
MODULE_DESCRIPTION("Barrier Mini-Firewall module");
MODULE_LICENSE("GPL");
