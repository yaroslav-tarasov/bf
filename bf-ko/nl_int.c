#include <linux/kernel.h>
#include <linux/module.h>
#include <net/sock.h>
#include <net/netlink.h>
#include "nl_int.h"
#include "base/hash_table.h"
#include "trx_data.h"

static struct sock *_nl_sock;
  

int find_rule(unsigned char* data);
void add_rule(struct filter_rule* fr);
void delete_rule(struct filter_rule* fr);
int nl_send_msg(struct sock * nl_sk,int destpid, int type, int flags,char* msg,int msg_size);
void list_rules(struct sock * nl_sk,int destpid);

DEFINE_MUTEX(nl_mutex);

static int s_rules_counter = 0;  

typedef struct thrd_params{
	// struct sock * nl_sk;
	int pid;
} tp_t;

tp_t thrd_params;

struct completion comp;

static int 
thread( void * data ) {
	int pid = ((tp_t*)data)->pid;

	// struct task_struct *curr = current; /* current - указатель на дескриптор текущей задачи */
        
	daemonize("thread");
        // allow_signal(SIGKILL);
	
	list_rules(_nl_sock,pid);

    	printk(KERN_INFO "%s skb->pid: %d\n",__func__,pid);
    	
	printk(KERN_INFO "Leave %s \n",__func__);
	return 0;
}

void 
wfc(void){
	wait_for_completion_timeout(&comp,1 * HZ);
	// wait_for_completion(&comp);
}

static int
nl_rcv_msg(struct sk_buff *skb, struct nlmsghdr *nlh)
{
    int type;
    unsigned char *data;
    struct nlmsgerr *nlerr;
    
    type = nlh->nlmsg_type;
    
    printk("%s nlh->nlmsg_type = %d ",__func__,type);
    
    switch (type)
    {
	case MSG_ADD_RULE:
	        data = NLMSG_DATA(nlh);
                if(find_rule((unsigned char*)&((filter_rule_t*)data)->base_rule)==0)
			printk("%s we have this rule ",__func__);
		else{
			printk("%s new rule added ",__func__);
			add_rule((filter_rule_t*)data);
			s_rules_counter++;
		}
		((filter_rule_t*)data)->id = s_rules_counter;
		printk("%s from netlink  TID %d index: %d src port: %d  dst_port: %d d_addr: %d s_addr: %d proto: %d\n",__func__,(int)current->pid,
			/*((filter_rule_t*)data)->id*/s_rules_counter,((filter_rule_t*)data)->base_rule.src_port,
			((filter_rule_t*)data)->base_rule.dst_port,((filter_rule_t*)data)->base_rule.d_addr.addr,
			((filter_rule_t*)data)->base_rule.s_addr.addr,((filter_rule_t*)data)->base_rule.proto);
		
		printk("%s index: %d ",	__func__,s_rules_counter);	
		break;
        case MSG_DELETE_RULE:
		printk("%s  --------------MSG_DELETE_RULE\n",__func__);
	        data = NLMSG_DATA(nlh);
                if(find_rule((unsigned char*)&((filter_rule_t*)data)->base_rule)==0){
			printk("%s delete rule ",__func__);
			delete_rule((filter_rule_t*)data);
		}
		else{
			printk("%s we don't have this rule ",__func__);
			
		}

		break;
        case MSG_GET_RULES: 
		printk("%s  --------------MSG_GET_RULES\n",__func__);
		data = NLMSG_DATA(nlh);
		//((filter_rule_t*)data)->base_rule.src_port=1200;
		// memset(data,1,sizeof(filter_rule_t));	
		// nl_send_msg(_nl_sock,skb,data,sizeof(filter_rule_t));
		//list_rules(_nl_sock,skb);
		printk(KERN_INFO "%s skb->pid: %d\n",__func__,nlh->nlmsg_pid);
                // thrd_params.nl_sk = _nl_sock;
		thrd_params.pid = nlh->nlmsg_pid;
		 /*hello_thread_id =*/ kernel_thread(thread, &thrd_params, CLONE_FS | CLONE_FILES | CLONE_SIGHAND | SIGCHLD | CLONE_KERNEL);
		// printk("%s: nl_send_msg %d\n", __func__, sizeof(filter_rule_t));
		break;
        case MSG_DONE:
		printk("%s  --------------MSG_DONE\n",__func__);
 		//s_rules_counter = 0;
		break;
        case MSG_OK:
		printk("%s  --------------MSG_OK\n",__func__);
 		//s_rules_counter = 0;
		printk(KERN_INFO "Got the flag NLM_F_ACK, nlh->nlmsg_flags =%d\n",nlh->nlmsg_flags);
		//if(nlh->nlmsg_flags&NLM_F_ACK)
		{
			complete(&comp);	
		}
		break;
        case NLMSG_ERROR:
		printk("%s  --------------NLMSG_ERROR\n",__func__);
		nlerr = (struct nlmsgerr*)NLMSG_DATA(nlh);
		//printf("Got some error: %d \n",nlerr->error);
		//complete(&comp);
		break;
	default:
		printk("%s: expect something else got %#x\n", __func__, type);
	        return -EINVAL;
    }

//    printk("%s: %02x %02x %02x %02x %02x %02x %02x %02x\n", __func__,
//            data[0], data[1], data[2], data[3],
//            data[4], data[5], data[6], data[7]);

//    printk("%s: %02x %02x \n", __func__,
//            *(unsigned int*)&data[0],
//            *(unsigned int*)&data[4]);
    printk(KERN_INFO "Leave %s \n",__func__);
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
    // int msg_size;
    // char *msg="Hello from kernel";
    int res;

    printk(KERN_INFO "Entering: %s\n", __FUNCTION__);
    
    // msg_size=strlen(msg);
    
    //nlh=(struct nlmsghdr*)skb->data;
    //printk(KERN_INFO "Netlink received msg payload: %s\n",(char*)nlmsg_data(nlh));
    pid = destpid; // nlh->nlmsg_pid; /*pid of sending process */

    skb_out = nlmsg_new(msg_size,0);

    if(!skb_out)
    {
        printk(KERN_ERR "Failed to allocate new skb\n");
        return -1;
    } 
    nlh=nlmsg_put(skb_out,0,0,/*NLMSG_DONE*/type,msg_size,flags);

    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    memcpy(nlmsg_data(nlh),msg,msg_size);
    
    res=nlmsg_unicast(nl_sk,skb_out,pid);
    
    if(res<0)
    {	
        printk(KERN_INFO "Error while sending message to user err=%d msg_size=%d pid=%d\n",res,msg_size,pid);
 	// kfree_skb(skb_out); Видимо только тогда когда не вызывается unicast вообще и наверное тогда
 	// nlmsg_free(skb_out);
    }

   // nla_put_u8(skb_out, 8, 9);
     // nlmsg_data(nlh);
    return res;
}

/*static*/ int
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

/*static*/ void
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

EXPORT_SYMBOL( nl_init );
EXPORT_SYMBOL( nl_exit );
