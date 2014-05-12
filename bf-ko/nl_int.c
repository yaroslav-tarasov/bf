#include <linux/kernel.h>
#include <linux/module.h>
#include <net/sock.h>
#include <net/netlink.h>
#include "nl_int.h"
#include "base/hash_table.h"
#include "trx_data.h"
#include "bf_config.h"

static struct sock *_nl_sock=NULL;

struct sock *  get_nl_sock(void){
	return _nl_sock;
}  

extern struct nf_bf_filter_config bf_config;


DEFINE_MUTEX(nl_mutex);

static int s_rules_counter = 0;  

typedef struct thrd_params{
	// struct sock * nl_sk;
	pid_t pid;
} tp_t;

tp_t thrd_params = {
	.pid = 0
};

pid_t get_client_pid(void){
	return thrd_params.pid;
}

struct completion comp;

static int 
thread( void * data ) {
	int pid = ((tp_t*)data)->pid;

	// struct task_struct *curr = current; /* current - указатель на дескриптор текущей задачи */
    // daemonize("thread");
    // allow_signal(SIGKILL);
	
	list_rules(_nl_sock,pid);

    PRINTK_DBG(KERN_INFO "%s skb->pid: %d\n",__func__,pid);
    PRINTK_DBG(KERN_INFO "Leave %s \n",__func__);
	return 0;
}

void 
wfc(void){
	wait_for_completion_timeout(&comp,1 * HZ);
}

static inline void
set_chain_policy(filter_rule_t* fr )
{
    if(fr->base.chain == CHAIN_INPUT)
        bf_config.chain_rule[0] = fr->policy;
    else if(fr->base.chain == CHAIN_OUTPUT)
        bf_config.chain_rule[1] = fr->policy;
    else if(fr->base.chain == CHAIN_ALL)
    {
        bf_config.chain_rule[0] = fr->policy;
        bf_config.chain_rule[1] = fr->policy;
    }
    else
        printk(KERN_ERR "%s Have no such chain\n",__func__);
}

static int
nl_rcv_msg(struct sk_buff *skb, struct nlmsghdr *nlh)
{
    int type;
    unsigned char *data;
    struct nlmsgerr *nlerr;
    filter_rule_list_t* fl;

    type = nlh->nlmsg_type;
    thrd_params.pid = nlh->nlmsg_pid;

    PRINTK_DBG("%s nlh->nlmsg_type = %d ",__func__,type);
    
    switch (type)
    {
	case MSG_ADD_RULE:
        data = NLMSG_DATA(nlh);

        if(find_rule((unsigned char*)&((filter_rule_t*)data)->base,NULL)==0){
                    PRINTK_DBG("%s we have this rule ",__func__);
        }
		else{
            PRINTK_DBG("%s new rule added ",__func__);
			add_rule((filter_rule_t*)data);
			s_rules_counter++;
		}
		((filter_rule_t*)data)->id = s_rules_counter;
        PRINTK_DBG("%s from netlink  TID %d index: %d src port: %d  dst_port: %d d_addr: %d s_addr: %d proto: %d\n",__func__,(int)current->pid,
            s_rules_counter,((filter_rule_t*)data)->base.src_port,
            ((filter_rule_t*)data)->base.dst_port,((filter_rule_t*)data)->base.d_addr.addr,
            ((filter_rule_t*)data)->base.s_addr.addr,((filter_rule_t*)data)->base.proto);
		
		break;
    case MSG_UPDATE_RULE:

        data = NLMSG_DATA(nlh);

        if(find_rule((unsigned char*)&((filter_rule_t*)data)->base,&fl)==0){
             PRINTK_DBG("%s we have this rule ",__func__);
             if (((filter_rule_t*)data)->off != SWITCH_NONE) fl->fr.off = ((filter_rule_t*)data)->off;
             if (((filter_rule_t*)data)->policy != POLICY_NONE) fl->fr.policy = ((filter_rule_t*)data)->policy;
        }
        else{
            PRINTK_DBG("%s we do not have rule  ",__func__);
        }
        // ((filter_rule_t*)data)->id = s_rules_counter;
        PRINTK_DBG("%s from netlink  TID %d src port: %d  dst_port: %d d_addr: %d s_addr: %d proto: %d\n",__func__,(int)current->pid,
            ((filter_rule_t*)data)->base.src_port,
            ((filter_rule_t*)data)->base.dst_port,((filter_rule_t*)data)->base.d_addr.addr,
            ((filter_rule_t*)data)->base.s_addr.addr,((filter_rule_t*)data)->base.proto);

        break;
    case MSG_DELETE_RULE:
        PRINTK_DBG("%s  --------------MSG_DELETE_RULE\n",__func__);
            data = NLMSG_DATA(nlh);
                if(find_rule((unsigned char*)&((filter_rule_t*)data)->base,NULL)==0){
            PRINTK_DBG("%s delete rule ",__func__);
            delete_rule((filter_rule_t*)data);
        }
        else{
            PRINTK_DBG("%s we don't have this rule ",__func__);

        }

        break;
    case MSG_DELETE_ALL_RULES:
        PRINTK_DBG("%s  --------------MSG_DELETE_ALL_RULES\n",__func__);
		delete_rules();

		break;
    case MSG_CHAIN_POLICY:
        data = NLMSG_DATA(nlh);
        if(data)
            set_chain_policy(((filter_rule_t*)data));

        break;
    case MSG_LOG_SUBSCRIBE:
        PRINTK_DBG("%s  --------------MSG_LOG_SUBSCRIBE",__func__);
        data = NLMSG_DATA(nlh);
        bf_config.pid_log = nlh->nlmsg_pid;//((_log_subscribe_msg_t*)data)->pid;
        atomic_set(&bf_config.init, 1);

        break;
    case MSG_GET_RULES:
        PRINTK_DBG("%s  --------------MSG_GET_RULES\n",__func__);
		data = NLMSG_DATA(nlh);
        PRINTK_DBG(KERN_INFO "%s skb->pid: %d\n",__func__,nlh->nlmsg_pid);
		thrd_params.pid = nlh->nlmsg_pid;
        kernel_thread(thread, &thrd_params, CLONE_FS | CLONE_FILES | CLONE_SIGHAND | SIGCHLD | CLONE_KERNEL);
		break;
        case MSG_DONE:
		printk("%s  --------------MSG_DONE\n",__func__);
 		//s_rules_counter = 0;
		break;
    case MSG_OK:
        PRINTK_DBG("%s  --------------MSG_OK\n",__func__);
        PRINTK_DBG(KERN_INFO "Got the flag NLM_F_ACK, nlh->nlmsg_flags =%d\n",nlh->nlmsg_flags);
		//if(nlh->nlmsg_flags&NLM_F_ACK)
		{
			complete(&comp);	
		}
		break;
    case NLMSG_ERROR:
        PRINTK_DBG("%s  --------------NLMSG_ERROR\n",__func__);
		nlerr = (struct nlmsgerr*)NLMSG_DATA(nlh);
		break;
	default:
		printk("%s: expect something else got %#x\n", __func__, type);
	        return -EINVAL;
    }

    PRINTK_DBG(KERN_INFO "Leave %s \n",__func__);
    return 0;
}

static void
nl_skb_rcv_msg(struct sk_buff *skb)
{
    mutex_lock(&nl_mutex);
    netlink_rcv_skb(skb, &nl_rcv_msg);
    mutex_unlock(&nl_mutex);
}

int  
nl_send_msg(struct sock * nl_sk,int destpid, int type, int flags,char* msg,int msg_size)
{
    struct nlmsghdr *nlh;
    int pid;
    struct sk_buff *skb_out;
    int res;

    //DL5 PRINTK_DBG(KERN_INFO "Entering: %s\n", __FUNCTION__);
    

    pid = destpid;

    skb_out = nlmsg_new(msg_size,0);

    if(!skb_out)
    {
        printk(KERN_ERR "Failed to allocate new skb\n");
        return -1;
    } 
    nlh=nlmsg_put(skb_out,0,0,/*NLMSG_DONE*/type,msg_size,flags);

    if(nlh){
        NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
        memcpy(nlmsg_data(nlh),msg,msg_size);
    }

    res=nlmsg_unicast(nl_sk,skb_out,pid);
    
    if(res<0)
    {	
        printk(KERN_ERR "Error while sending message to user err=%d msg_size=%d pid=%d\n",res,msg_size,pid);
    }


    return res;
}

int
nl_send_lst(struct sock * nl_sk,int destpid,  filter_rule_list_t* lst,int lst_size,int* end_list)
{
    struct nlmsghdr *nlh=NULL;
    struct filter_rule_list *a_rule=NULL;
    int pid;
    struct sk_buff *skb_out = NULL;
    int res=0,flags=0,i=0,msg_size=sizeof(filter_rule_t),msg_cnt=1;

    if(!lst_size)
        lst_size = NLMSG_DEFAULT_SIZE / (msg_size + NLMSG_HDRLEN);

    PRINTK_DBG(KERN_INFO "Entering: %s\n", __FUNCTION__);

    pid = destpid;

    if (lst_size <= 0) return -1;

    list_for_each_entry(a_rule, &lst->full_list, full_list) {

        if (i==0 || i == lst_size || !nlh || !skb_out)
            skb_out = nlmsg_new(NLMSG_DEFAULT_SIZE,0);

        if(msg_cnt%2==0 && i == lst_size-1) { flags = NLM_F_ACK; };

        if(!skb_out)
        {
            printk(KERN_ERR "Failed to allocate new skb\n");
            return -1;
        }

        nlh=nlmsg_put(skb_out,0,0,MSG_DATA,msg_size,flags);

        if(nlh){
            NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
            memcpy(nlmsg_data(nlh),&a_rule->fr,msg_size);
            PRINTK_DBG(KERN_INFO "%s #%d Src_addr: %X; dst_addr: %X; proto: %d; src_port: %d dst_port: %d\n", __FUNCTION__, i,
                    a_rule->fr.base.s_addr.addr, a_rule->fr.base.d_addr.addr,
                    a_rule->fr.base.proto, a_rule->fr.base.src_port, a_rule->fr.base.dst_port);
        }

        if(flags == NLM_F_ACK) {
            flags = 0;
        }

        if(++i == lst_size && skb_out){
            res=nlmsg_unicast(nl_sk,skb_out,pid);
            PRINTK_DBG(KERN_INFO " %s:  nlmsg_unicast\n", __FUNCTION__);
            skb_out = NULL;
            i = 0;
            if(res<0)
            {
                printk(KERN_ERR "Error while sending message to user err=%d msg_size=%d pid=%d\n",res,msg_size,pid);
            }

            if(msg_cnt++%2==0) {
                wfc();
            }
        }

    }

    if (skb_out)
        res=nlmsg_unicast(nl_sk,skb_out,pid);

    if(res<0)
    {
        printk(KERN_ERR "Error while sending message to user err=%d msg_size=%d pid=%d\n",res,msg_size,pid);
    }


    return res;
}

int
nl_init(void)
{
    _nl_sock = netlink_kernel_create(&init_net, NETLINK_USERSOCK, 0,
            nl_skb_rcv_msg, NULL, THIS_MODULE);
    if (!_nl_sock) {
        printk(KERN_ERR "%s: receive handler registration failed\n", __func__);
        return -ENOMEM;
    }
    
    init_completion(&comp);
    return 0;
}

void
nl_exit(void)
{
    if (_nl_sock) {
        netlink_kernel_release(_nl_sock);
    }
}

void input (struct sock *sk, int len)
{
  wait_queue_head_t w;
  wake_up_interruptible(&w/*sk->sleep*/);
	wake_up_interruptible(sk->sk_sleep);
}
