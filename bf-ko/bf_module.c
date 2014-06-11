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
#include <linux/if_vlan.h>
#include <linux/proc_fs.h>  /* Necessary because we use the proc fs */
#include <linux/list.h>
#include <net/ip.h>
#include <asm/uaccess.h>	/* For copy_from_user  */
#include "base/hash_table.h"
#include "nl_int.h"
#include "../common/trx_data.h"
#include "bf_config.h"


struct nf_bf_filter_config bf_config = { .init = ATOMIC_INIT(0),
                                         .pid_log=0
                                       };

//DEFINE_SPINLOCK(list_mutex);	

#define ACK_EVERY_N_MSG  50
#define bf_filter_name "bf-filter"


int fdebug=1;

static struct filter_rule_list lst_fr;
static struct filter_rule_list lst_fr_in;
static struct filter_rule_list lst_fr_out;
struct hash_table map_fr;

static  void init_rules(void)
{    

    bf_config.chain_policy[INPUT] = NF_ACCEPT;
    bf_config.chain_policy[OUTPUT] = NF_ACCEPT;
    
    hash_table_init(&map_fr, 10, NULL);

#if 0
    struct filter_rule_list  *a_rule;
    int i;
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
    a_new_fr->fr.off = SWITCH_NO;
    //INIT_LIST_HEAD(&a_new_fr->full_list);
    // add the new node to mylist
    list_add(&(a_new_fr->full_list), &(lst_fr.full_list));//list_add_tail(&(a_new_fr->list), &(lst_fr.list));
	hash_table_insert(&map_fr, &a_new_fr->entry, (const char*)&a_new_fr->fr.base_rule, sizeof(struct filter_rule_base));
	
	if(a_new_fr->fr.base_rule.proto == IPPROTO_UDP)
		list_add(&(a_new_fr->protocol_list), &(lst_fr_udp.protocol_list));
	else if (a_new_fr->fr.base_rule.proto == IPPROTO_TCP)
		list_add(&(a_new_fr->protocol_list), &(lst_fr_tcp.protocol_list));		
	
    }

    i =0;
    list_for_each_entry(a_rule, &lst_fr.full_list, full_list) {
        printk(KERN_INFO "#%d Src_addr: %X; dst_addr: %X; proto: %d; src_port: %d dst_port: %d\n", i++,a_rule->fr.base_rule.s_addr.addr, a_rule->fr.base_rule.d_addr.addr, a_rule->fr.base_rule.proto, a_rule->fr.base_rule.src_port, a_rule->fr.base_rule.dst_port);
    
     }
#endif

}

inline int sorted(struct filter_rule* fr1,struct filter_rule* fr2)
{
	int w1=0,w2=0;	
	
    fr1->base.src_port==0?w1++:0;
        fr2->base.src_port==0?w2++:0;

    fr1->base.dst_port==0?w1++:0;
        fr2->base.dst_port==0?w2++:0;

    fr1->base.s_addr.addr==0?w1++:0;
        fr2->base.s_addr.addr==0?w2++:0;

    fr1->base.d_addr.addr==0?w1++:0;
        fr2->base.d_addr.addr==0?w2++:0;
		

	return w1 - w2;
}

void add_entry_sort(struct filter_rule_list *new,struct filter_rule_list *rule_list)
{
    struct list_head *ptr;
    struct  filter_rule_list *entry;

    list_for_each(ptr, &rule_list->chain_list) {
        entry = list_entry(ptr, struct filter_rule_list, chain_list);
        if (sorted(&entry->fr , &new->fr)<0) {
            list_add_tail_rcu(&new->chain_list, ptr);
            return;
        }
    }
    list_add_tail_rcu(&new->chain_list, &rule_list->chain_list);
}

void add_rule(struct filter_rule* fr)
{
	struct filter_rule_list *a_new_fr;
	a_new_fr = kmalloc(sizeof(*a_new_fr), GFP_KERNEL);	
	memcpy(&a_new_fr->fr,fr,sizeof(filter_rule_t));

//spin_lock(&list_mutex);
    list_add_tail_rcu(&(a_new_fr->full_list), &(lst_fr.full_list));//list_add_tail(&(a_new_fr->list), &(lst_fr.list));

    hash_table_insert(&map_fr, &a_new_fr->entry, (const char*)&a_new_fr->fr.base, sizeof(struct filter_rule_base));
    if(a_new_fr->fr.base.chain == CHAIN_INPUT)
#ifndef TEST_SORT_ADD    
        list_add_tail_rcu(&(a_new_fr->chain_list), &(lst_fr_in.chain_list));
#else		
		add_entry_sort(a_new_fr,&lst_fr_in);
#endif		
    else if (a_new_fr->fr.base.chain == CHAIN_OUTPUT)
#ifndef TEST_SORT_ADD     
        list_add_tail_rcu(&(a_new_fr->chain_list), &(lst_fr_out.chain_list));
#else		
		add_entry_sort(a_new_fr,&lst_fr_out);
#endif
//spin_unlock(&list_mutex);
	
}



inline int cmp_rule(struct filter_rule_base* fr1,struct filter_rule_base* fr2)
{
	return memcmp(fr1,fr2,sizeof(struct filter_rule_base));
}

void free_rule(struct rcu_head *rh)
{
	struct filter_rule_list *a_rule;
	a_rule = container_of(rh, struct filter_rule_list, rcu);
	kfree(a_rule);

    PRINTK_DBG(KERN_INFO "delete_rule : call_rcu : free_rule\n");
}

void delete_rule(struct filter_rule* fr)
{
	struct filter_rule_list *a_rule, *tmp;	

    PRINTK_DBG(KERN_INFO "Enter delete_rule  \n");

//spin_lock(&list_mutex);
	list_for_each_entry_safe(a_rule, tmp, &lst_fr_in.chain_list, chain_list){
        if(cmp_rule(&a_rule->fr.base,&fr->base)==0)
		{		
			list_del_rcu(&a_rule->chain_list);
			break;
		}
	}

	list_for_each_entry_safe(a_rule, tmp, &lst_fr_out.chain_list, chain_list){
        if(cmp_rule(&a_rule->fr.base,&fr->base)==0)
		{		
			list_del_rcu(&a_rule->chain_list);
			break;
		}
	}

    hash_table_del_key_safe(&map_fr,(const char*)&fr->base, sizeof(struct filter_rule_base));

	list_for_each_entry_safe(a_rule, tmp, &lst_fr.full_list, full_list){

        if(cmp_rule(&a_rule->fr.base,&fr->base)==0){
			list_del_rcu(&a_rule->full_list);
			// kfree(a_rule);

            PRINTK_DBG(KERN_INFO "delete_rule : call_rcu\n");

			call_rcu(&a_rule->rcu, free_rule);
		}
	}

    PRINTK_DBG(KERN_INFO "Leave delete_rule  \n");
//spin_unlock(&list_mutex);	
	
}

void delete_rules(void)
{ 
    struct filter_rule_list *a_rule, *tmp;
//    unsigned int hti = 0;

//    struct hash_entry *hentry;
//    struct list_head * pos;
      
    list_for_each_entry_safe(a_rule, tmp, &lst_fr_in.chain_list, chain_list){
         list_del_rcu(&a_rule->chain_list);
    }

    list_for_each_entry_safe(a_rule, tmp, &lst_fr_out.chain_list, chain_list){
         list_del_rcu(&a_rule->chain_list);
    }

    //hash_table_for_each_safe(hentry, &map_fr, pos, hti){
    //	hash_table_del_hash_entry_safe(&map_fr,hentry);
    //}

    list_for_each_entry_safe(a_rule, tmp, &lst_fr.full_list, full_list){
        // printk(KERN_INFO "freeing node %s\n", a_rule->name);
        hash_table_del_key_safe(&map_fr,(const char*)&a_rule->fr.base, sizeof(struct filter_rule_base));
        list_del_rcu(&a_rule->full_list);
        // kfree(a_rule);
        call_rcu(&a_rule->rcu, free_rule);
    }
}

void cleanup_rules(void)
{ 
    struct filter_rule_list *a_rule, *tmp;
        
    list_for_each_entry_safe(a_rule, tmp, &lst_fr_in.chain_list, chain_list){
         list_del(&a_rule->chain_list);
    }

    list_for_each_entry_safe(a_rule, tmp, &lst_fr_out.chain_list, chain_list){
         list_del(&a_rule->chain_list);
    }

    // hash_table_finit(&map_fr);

    list_for_each_entry_safe(a_rule, tmp, &lst_fr.full_list, full_list){
         // printk(KERN_INFO "freeing node %s\n", a_rule->name);
         hash_table_del_key_safe(&map_fr,(const char*)&a_rule->fr.base, sizeof(struct filter_rule_base));
         list_del(&a_rule->full_list);
         kfree(a_rule);

    }
}

void list_rules(struct sock * nl_sk, int destpid, filter_rule_t* pfr_pattern)
{

    int ret;
#if 1
    struct filter_rule_list *a_rule;
    int i=0,flags = 0;

    // Сначала отправляем политику для цепочки INPUT
    {
        filter_rule_t fr;
        memset(&fr,0,sizeof(filter_rule_t));
        fr.policy = get_policy(bf_config.chain_policy[INPUT]);
        fr.base.chain = CHAIN_INPUT;
        ret=nl_send_msg(nl_sk,destpid, MSG_DATA, 0,(char*)&fr,sizeof(fr));
        if(ret<0)
             return;
    }

    // Далее отправляем политику для цепочки OUTPUT
    {
        filter_rule_t fr;
        memset(&fr,0,sizeof(filter_rule_t));
        fr.policy = get_policy(bf_config.chain_policy[OUTPUT]);
        fr.base.chain = CHAIN_OUTPUT;
        ret=nl_send_msg(nl_sk,destpid, MSG_DATA, 0,(char*)&fr,sizeof(fr));
        if(ret<0)
             return;
    }

    // Далее правила цепочки INPUT

    list_for_each_entry(a_rule, &lst_fr_in.chain_list, chain_list) {
        if(pfr_pattern->base.chain==CHAIN_ALL?true:fr_pattern(&a_rule->fr,pfr_pattern)){

            if(++i%ACK_EVERY_N_MSG==0) { flags = NLM_F_ACK; };

            PRINTK_DBG(KERN_INFO "#%d Src_addr: %X; dst_addr: %X; proto: %d; src_port: %d dst_port: %d\n", i,
                    a_rule->fr.base.s_addr.addr, a_rule->fr.base.d_addr.addr,
                    a_rule->fr.base.proto, a_rule->fr.base.src_port, a_rule->fr.base.dst_port);

            a_rule->fr.id = i;
            ret=nl_send_msg(nl_sk,destpid, MSG_DATA, flags,(char*)&a_rule->fr,sizeof(a_rule->fr));
            if(ret<0)
                 return;

            if(flags == NLM_F_ACK) {
                flags = 0; wfc();
            }
        }
    }

    // Далее правила цепочки OUTPUT

    list_for_each_entry(a_rule, &lst_fr_out.chain_list, chain_list) {
        if(pfr_pattern->base.chain==CHAIN_ALL?true:fr_pattern(&a_rule->fr,pfr_pattern)){

            if(++i%ACK_EVERY_N_MSG==0) { flags = NLM_F_ACK; };

            PRINTK_DBG(KERN_INFO "#%d Src_addr: %X; dst_addr: %X; proto: %d; src_port: %d dst_port: %d\n", i,
                    a_rule->fr.base.s_addr.addr, a_rule->fr.base.d_addr.addr,
                    a_rule->fr.base.proto, a_rule->fr.base.src_port, a_rule->fr.base.dst_port);

            a_rule->fr.id = i;
            ret=nl_send_msg(nl_sk,destpid, MSG_DATA, flags,(char*)&a_rule->fr,sizeof(a_rule->fr));
            if(ret<0)
                 return;

            if(flags == NLM_F_ACK) {
                flags = 0; wfc();
            }
        }
    }



    {
        msg_done_t msg;
        msg.counter = i;
        nl_send_msg(nl_sk,destpid, MSG_DONE, 0, (char*)&msg,sizeof(msg_done_t));
    }
#else
    int end_list =0;
    filter_rule_t fr;
    ret= nl_send_lst( nl_sk,destpid,  &lst_fr,20,&end_list);

    if(ret<0)
         return;

    msg_done_t msg;
    msg.counter = ret;
    nl_send_msg(nl_sk,destpid, MSG_DONE, 0, (char*)&msg,sizeof(msg_done_t));
#endif

}

int find_rule(unsigned char* data,struct filter_rule_list **res)
{
    struct hash_entry *hentry;
	if ((hentry =
	     hash_table_lookup_key(&map_fr, data,
				   sizeof(filter_rule_base_t))) == NULL) {
        if(res) *res = NULL;
	return -1;
	} else {
		/* just like the listr_item() */
		struct filter_rule_list *tmp;
		tmp = hash_entry(hentry, struct filter_rule_list, entry);
        if (res) *res=tmp;
	return 0;	
	}
}

static inline __be16 vlan_proto(const struct sk_buff *skb)
{
         if (vlan_tx_tag_present(skb))
                 return skb->protocol;
         else if (skb->protocol == htons(ETH_P_8021Q))
                 return vlan_eth_hdr(skb)->h_vlan_encapsulated_proto;
         else
                 return 0;
}

static void dequeue_rule_packet(struct nf_bf_filter_config *nwf_list, struct sk_buff_head *sg_skb_list)
{
    struct  sk_buff *skb;

    skb = __skb_dequeue(nwf_list->skb_list);
    if (!skb)
        return;

    kfree_skb(skb);
}

static void add_to_skb_list(struct nf_bf_filter_config *nwf_list, struct sk_buff *skb)
{
    struct  sk_buff_head *skb_list;

    skb_list = nwf_list->skb_list;

    skb = skb_get(skb);
    skb_orphan(skb);
    skb_queue_tail(skb_list, skb);

    queue_work(bf_config.wq_logging, &bf_config.work_logging);
}



static void work_handler(struct work_struct * work) {

    struct nf_bf_filter_config* config;
    struct iphdr       *ip_header=NULL;        // IP header struct
    struct tcphdr      *tcp_header=NULL;	// TCP Header
    struct udphdr      *udp_header=NULL;      // UDP header struct
    struct  sk_buff *skb;

    filter_rule_t fr;
    pid_t destpid;

    config = container_of(work, struct nf_bf_filter_config, work_logging);

    //dequeue_rule_packet(struct nf_bf_filter_config *nwf_list, struct sk_buff_head *sg_skb_list)

    skb = __skb_dequeue(config->skb_list);
    if (!skb)
        return;

    if(atomic_read(&bf_config.init)>0)
    {
        destpid = bf_config.pid_log;//get_client_pid();
        memset(&fr,0,sizeof(filter_rule_t));
	
        ip_header = (struct iphdr *) skb_network_header(skb);
        udp_header = (struct udphdr *)(skb_transport_header(skb) + ip_hdrlen(skb));
        tcp_header = (struct tcphdr *)(skb_transport_header(skb) + ip_hdrlen(skb));

        fr.base.proto = ip_header->protocol;
        fr.base.src_port =  ntohs(ip_header->protocol==IPPROTO_UDP? udp_header->source:tcp_header->source);
        fr.base.dst_port =  ntohs(ip_header->protocol==IPPROTO_UDP? udp_header->dest:tcp_header->dest);
        fr.base.s_addr.addr = ip_header->saddr;
        fr.base.d_addr.addr = ip_header->daddr;

//      PRINTFR(fr);

        if(destpid)
            nl_send_msg(get_nl_sock(),destpid, MSG_LOG, 0, (char*)&fr,sizeof(fr));
    }

    kfree_skb(skb);
}

static struct proc_dir_entry *skb_filter;
 
static int filter_value = 1;


#define goto_target(target) { rcu_read_unlock(); \
          return target; }


int apply_policy(enum bf_policy_t p)
{
    switch(p)
    {
    case POLICY_ACCEPT:
        return NF_ACCEPT;
    case POLICY_DROP:
        return NF_DROP;
    default:   // POLICY_NONE
        return NF_DROP;
    }
}

enum bf_policy_t get_policy(int p)
{
    switch(p)
    {
    case NF_ACCEPT:
        return POLICY_ACCEPT;
    case NF_DROP:
        return POLICY_DROP;
    default:   // POLICY_NONE
        return POLICY_NONE;
    }
}
unsigned int hook_func(unsigned int hooknum, 
            struct sk_buff *skb, 
            const struct net_device *in, 
            const struct net_device *out, 
            int (*okfn)(struct sk_buff *))
{
    struct sk_buff     *sock_buff;
    struct udphdr      *udp_header=NULL;      // UDP header struct
    struct iphdr       *ip_header=NULL;        // IP header struct
    struct icmphdr     *icmp_header=NULL;	// ICMP Header
    struct tcphdr      *tcp_header=NULL;	// TCP Header
    struct ethhdr      *ethheader=NULL;      // Ethernet Header
    struct vlan_ethhdr *vlan_header=NULL;
    struct list_head   *chain_list;
    struct filter_rule_list  *a_rule;

    sock_buff = skb;	
 
    ethheader = (struct ethhdr*) skb_mac_header(sock_buff); 
    ip_header = (struct iphdr *) skb_network_header(sock_buff);
    if (vlan_proto(sock_buff))
    	vlan_header  = (struct vlan_ethhdr *)vlan_eth_hdr(sock_buff);
 
    if(!sock_buff || !ip_header || !ethheader || filter_value==0)
        return NF_ACCEPT;

    chain_list = in!=0?&lst_fr_in.chain_list:&lst_fr_out.chain_list;

/// Работаем только с TCP и UDP все остальные  пакеты пропускаем

    if(ip_header->protocol == IPPROTO_UDP || ip_header->protocol == IPPROTO_TCP){
    
    rcu_read_lock();
    
    list_for_each_entry_rcu(a_rule, chain_list /*&lst_fr_in.chain_list\*/, chain_list) {

    __u8 loc_policy = a_rule->fr.policy;


    if(ip_header->protocol == IPPROTO_UDP && a_rule->fr.base.proto == IPPROTO_UDP){
        udp_header = (struct udphdr *)(skb_transport_header(sock_buff) + ip_hdrlen(sock_buff));
        if(udp_header){
	    
            if( ( (a_rule->fr.base.src_port>0)?(ntohs(udp_header->source) == a_rule->fr.base.src_port):1) &&
                ((a_rule->fr.base.dst_port>0)?(ntohs(udp_header->dest) == a_rule->fr.base.dst_port):1) &&
                (a_rule->fr.base.s_addr.addr>0?(ntohl(ip_header->saddr) == a_rule->fr.base.s_addr.addr):1) &&
                (a_rule->fr.base.d_addr.addr>0?(ntohl(ip_header->saddr) == a_rule->fr.base.d_addr.addr):1) &&

                a_rule->fr.off==SW_NO){

                PRINTFR(a_rule->fr);

                //queue_work(bf_config.wq_logging, &bf_config.work_logging);
                add_to_skb_list( &bf_config, skb);

                goto_target(apply_policy(loc_policy));
            }


        }else
            goto_target(NF_DROP);
    } else  if(ip_header->protocol == IPPROTO_TCP && a_rule->fr.base.proto == IPPROTO_TCP){
        //PRINTK_DBG(KERN_INFO "---------- TCP -------------\n");
        tcp_header = (struct tcphdr *)(skb_transport_header(sock_buff) + ip_hdrlen(sock_buff));
        if(tcp_header){	

            if( (a_rule->fr.base.src_port>0?(ntohs(tcp_header->source) == a_rule->fr.base.src_port):1) &&
            (a_rule->fr.base.dst_port>0?(ntohs(tcp_header->dest) == a_rule->fr.base.dst_port):1) &&
            (a_rule->fr.base.s_addr.addr>0?(ntohs(ip_header->saddr) == a_rule->fr.base.s_addr.addr):1) &&
            (a_rule->fr.base.d_addr.addr>0?(ntohs(ip_header->saddr) == a_rule->fr.base.d_addr.addr):1) && a_rule->fr.off==SW_NO){
                PRINTFR(a_rule->fr);
                goto_target(apply_policy(loc_policy));
            }

        }else
            goto_target(NF_DROP);
    } else  if(ip_header->protocol == IPPROTO_ICMP){
        //PRINTK_DBG(KERN_INFO "---------- ICMP -------------\n");
        icmp_header = (struct icmphdr *)(skb_transport_header(sock_buff) + ip_hdrlen(sock_buff));
        /*if(icmp_header){
	    // printk(KERN_INFO "SRC: (%pM) --> DST: (%pM)\n",ethheader->h_source,ethheader->h_dest);

            if(ethheader && skb_mac_header_was_set(skb)) printk(KERN_INFO "SRC: (%pM) --> DST: (%pM)\n",ethheader->h_source,ethheader->h_dest); 

            PRINTK_DBG(KERN_INFO "SRC: (%u.%u.%u.%u) --> DST: (%u.%u.%u.%u)\n",NIPQUAD(ip_header->saddr),NIPQUAD(ip_header->daddr));
            PRINTK_DBG(KERN_INFO "ICMP type: %d - ICMP code: %d  in %s  out %s \n",icmp_header->type, icmp_header->code,in!=NULL?"true":"false",out!=NULL?"true":"false");
        }else
            goto_target(NF_DROP);//return NF_DROP;*/
    }
    
    
    } // end of list_for_each_entry

    rcu_read_unlock();


    return in!=0 ? bf_config.chain_policy[INPUT] : bf_config.chain_policy[OUTPUT];

    }

    return NF_ACCEPT;
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
    INIT_LIST_HEAD(&lst_fr_in.chain_list);	
    INIT_LIST_HEAD(&lst_fr_out.chain_list);
    INIT_RCU_HEAD(&lst_fr.rcu);
    INIT_RCU_HEAD(&lst_fr_in.rcu);
    INIT_RCU_HEAD(&lst_fr_out.rcu);
    
    bf_config.wq_logging = create_workqueue("do_logging");
    INIT_WORK(&(bf_config.work_logging), work_handler);

    bf_config.skb_list 	= (struct sk_buff_head*)kmalloc(sizeof(struct sk_buff_head), GFP_KERNEL);
    skb_queue_head_init(bf_config.skb_list);
    
    // mutex_init(&list_mutex);

    init_rules();
	
    skb_filter = create_proc_entry( bf_filter_name, 0644, NULL);
 
    // If we cannot create the proc entry
    if(skb_filter == NULL){
        ret = -ENOMEM;
        if( skb_filter )
            remove_proc_entry( bf_filter_name, &proc_root);
 
        printk(KERN_ERR "Barrier Mini-Firewall: Could not allocate memory.\n");
        goto error;
 
    }else{		
        skb_filter->read_proc = skb_read;
        skb_filter->write_proc = skb_write;
        //skb_filter->owner = THIS_MODULE;	
    }	
 
    // Netfilter hook information, specify where and when we get the SKB
    bf_config.nfho_out.hook = hook_func;
    // nfho.hooknum = NF_INET_PRE_ROUTING;
    bf_config.nfho_out.hooknum = NF_INET_POST_ROUTING;

    bf_config.nfho_out.pf = PF_INET;
    bf_config.nfho_out.priority = NF_IP_PRI_LAST;
    //nfho_out.priority = NF_IP_PRI_FIRST;
#if (LINUX_VERSION_CODE >= 0x020500) 
    bf_config.nfho_out.owner = THIS_MODULE;
#endif

    nf_register_hook(&bf_config.nfho_out);
 
    bf_config.nfho_in.hook = hook_func;
    bf_config.nfho_in.hooknum = NF_INET_PRE_ROUTING;
    // nfho.hooknum = NF_INET_POST_ROUTING;

    bf_config.nfho_in.pf = PF_INET;
    //nfho.priority = NF_IP_PRI_LAST;
    bf_config.nfho_in.priority = NF_IP_PRI_FIRST;
#if (LINUX_VERSION_CODE >= 0x020500) 
    bf_config.nfho_in.owner = THIS_MODULE;
#endif
    nf_register_hook(&bf_config.nfho_in);

    printk(KERN_INFO "Registering Barrier Mini-Firewall module\n");
    //
    nl_init();

error:
    return ret;
}
 
void cleanup_module()
{
    PRINTK_DBG(KERN_INFO " %s:  nf_unregister_hook\n", __FUNCTION__);
    nf_unregister_hook(&bf_config.nfho_in);
    nf_unregister_hook(&bf_config.nfho_out);

    PRINTK_DBG(KERN_INFO " %s:  remove_proc_entry\n", __FUNCTION__);
    if ( skb_filter )
        remove_proc_entry(bf_filter_name, NULL);

    PRINTK_DBG(KERN_INFO " %s:  flush_workqueue\n", __FUNCTION__);
	flush_workqueue(bf_config.wq_logging);
	destroy_workqueue(bf_config.wq_logging);  

    PRINTK_DBG(KERN_INFO " %s:  nl_exit\n", __FUNCTION__);
    nl_exit();

    // delete_rules();
    PRINTK_DBG(KERN_INFO " %s:  rcu_barrier\n", __FUNCTION__);
    rcu_barrier();
    //mutex_destroy(&list_mutex);

    PRINTK_DBG(KERN_INFO " %s:  cleanup_rules\n", __FUNCTION__);
    cleanup_rules();
    
    hash_table_finit(&map_fr);   

    printk(KERN_INFO "Unregistered the Barrier Mini-Firewall module\n");
}
 
MODULE_AUTHOR("Yaroslav Tarasov");
MODULE_DESCRIPTION("Barrier Mini-Firewall module");
MODULE_LICENSE("GPL");
