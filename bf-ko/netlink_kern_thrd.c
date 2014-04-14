/*
Program Description :: A thread based kernel module which main purpose is to send the ping packets from kernel space to User space via netlink socket. (keep default ping duration i.e.1Sec)
*/

/*
    *************************************************
    * Author            : Sudhansu Sekhar Mishra
    * Module           : Kernel Based Netlink Socket
    * OS version      : 2.6.9-55ELSmp
    * Type                : Open Source(GPL)
    * E_mail            : sudhansu.8454@gmail.com
    * File Name       : Netlink_Kernel_thread.c
    *************************************************
*/

/************ Kernel Header Files *************/
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>  
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/spinlock.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/completion.h>
#include <asm-i386/signal.h>
#include <asm/param.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/ctype.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <net/sock.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

/************ Module specific *************/
#define BUFSIZE 2048
#define TRIALS 10
#define TRUE 1
#define FALSE 0
#define NETLINK_HELLO 19
#define NETLINK_BYE 21

static struct sock *hello_sock_z = NULL;
static struct sock *bye_sock_z = NULL;

static int usr_pid_hello = 0;
static int usr_pid_bye = 0;

static int hello_thread_id = -1;
static int bye_thread_id = -1;

static DECLARE_WAIT_QUEUE_HEAD(event_wait_hello_z);
static DECLARE_WAIT_QUEUE_HEAD(event_wait_bye_z);

static int nf_registered_hello_ipv4;
static int nf_registered_bye_ipv4;
static int failed_to_send_hello = 0;
static int failed_to_send_bye = 0;

static spinlock_t send_hello_lock = SPIN_LOCK_UNLOCKED;
static spinlock_t send_bye_lock = SPIN_LOCK_UNLOCKED;

void register_IPv4_in_hook(void);

//NF HOOK for IPv4 incoming packets
unsigned int hello_IPv4_in_hook (unsigned int hook, 
                                 struct sk_buff **pskb,
                                   const struct net_device *indev,
                                   const struct net_device *outdev);
unsigned int bye_IPv4_in_hook (unsigned int hook, 
                                 struct sk_buff **pskb,
                                   const struct net_device *indev,
                                   const struct net_device *outdev);
static int send_bye_response( void *msg, int size );
static int send_hello_response( void *msg, int size );
int createThread_hello(void);
int createThread_bye(void);
static int process_hello_requests(void *data);
static int process_bye_requests(void *data);
static struct nf_hook_ops hello_IPv4_inHook_ops =
{
        {NULL, NULL},
        hello_IPv4_in_hook,        // hook function
        THIS_MODULE,            //owner
        PF_INET,                // protocol family for IPv4
        NF_IP_PRE_ROUTING,        // hook to be manipulated
        NF_IP_PRI_FILTER + 1    // priority
};
static struct nf_hook_ops bye_IPv4_inHook_ops =
{
        {NULL, NULL},
        bye_IPv4_in_hook,        // hook function
        THIS_MODULE,            //owner
        PF_INET,                // protocol family for IPv4
        NF_IP_PRE_ROUTING,      // hook to be manipulated
        NF_IP_PRI_FILTER + 1    // priority
};
unsigned int hello_IPv4_in_hook (unsigned int hook, 
                                 struct sk_buff **pskb,
                                   const struct net_device *indev,
                                   const struct net_device *outdev)
{
    int iLength;
    int i;

    char msg[BUFSIZE];
    unsigned char *p = NULL;

    struct iphdr *iph = NULL;
    
    memset(&msg, 0, BUFSIZE);
    iph = (struct iphdr *)(*pskb)->nh.iph;

    if (iph == NULL){
        printk(KERN_CRIT"hello_IPv4_in_hook :: Failed to decode ip header :: drop the pkt\n");
        return NF_DROP;
    }
    if (iph->protocol != 0x01)
        goto getout;
    iLength = ntohs(iph->tot_len);
    printk(KERN_CRIT"Hello, Protocol = %x\n",iph->protocol);
    printk(KERN_CRIT"Hello, packet length = %d\n",iLength);
    p = (unsigned char *)((*pskb)->data);
    
    if (p)
    {
        if (iLength < (BUFSIZE / 2)){
            for (i = 0; i < iLength; ++i){
                sprintf(msg + i * 2, "%02x", p[i]);
            }
        }
        else{
            for (i = 0; i < (BUFSIZE / 2); ++i)
            {
                sprintf(msg + i * 2, "%02x", p[i]);
            }
        }
        //printk(KERN_CRIT"Hello Payload = %s\n",msg);
    }
    spin_lock(&send_hello_lock);
    send_hello_response(msg, strlen(msg));
    spin_unlock(&send_hello_lock);

getout:
    return NF_ACCEPT;
}
unsigned int bye_IPv4_in_hook (unsigned int hook, 
                               struct sk_buff **pskb,
                               const struct net_device *indev,
                               const struct net_device *outdev)
{
        int iLength;
        int i;

        char msg[BUFSIZE];
        unsigned char *p = NULL;

        struct iphdr *iph = NULL;

        memset(&msg, 0, BUFSIZE);
        iph = (struct iphdr *)(*pskb)->nh.iph;

        if (iph == NULL){
                printk(KERN_CRIT"bye_IPv4_in_hook :: Failed to decode ip header :: drop the pkt\n");
                return NF_DROP;
        }
        if (iph->protocol != 0x01)
                goto getout;
        iLength = ntohs(iph->tot_len);
        printk(KERN_CRIT"BYE, Protocol = %x\n",iph->protocol);
        printk(KERN_CRIT"BYE, packet length = %d\n",iLength);
        p = (unsigned char *)((*pskb)->data);

        if (p)
        {
            if (iLength < (BUFSIZE / 2)){
                for (i = 0; i < iLength; ++i){
                    sprintf(msg + i * 2, "%02x", p[i]);
                }
            }
            else{
                for (i = 0; i < (BUFSIZE / 2); ++i)
                {
                    sprintf(msg + i * 2, "%02x", p[i]);
                }
            }
            //printk(KERN_CRIT"Bye Payload = %s\n",msg);
        }
        spin_lock(&send_bye_lock);
        send_bye_response(msg, strlen(msg));
        spin_unlock(&send_bye_lock);

getout:
        return NF_ACCEPT;
}

static int send_hello_response( void *msg, int size )
{
    int result;
    struct sk_buff *skb = NULL;
    struct nlmsghdr *nlh = NULL;
    if (0 == usr_pid_hello){
        //printk(KERN_CRIT"User spce Application is not ready\n");
        return -1;
    }
    skb = alloc_skb( NLMSG_SPACE( size ), GFP_ATOMIC );
    if( !skb )
    {
        printk(KERN_CRIT"send_hello_response :: Failed to allocate memory\n");
        return -ENOMEM;
    }
    result = -EINVAL;
    nlh = NLMSG_PUT( skb, 0, 0, 0, size );
    memset(nlh, 0, size);
    memcpy( NLMSG_DATA( nlh ), msg, size );
    if (failed_to_send_hello)
        goto nlmsg_failure;
    if ((hello_sock_z != NULL) && (usr_pid_hello != 0)){
        //Sudhansu :: on failure netlink_unicast will free the buffer (skb)
        result = netlink_unicast( hello_sock_z, skb, usr_pid_hello, 0 );
    }
    else{
        printk(KERN_CRIT"Hello >> Comunication between U-Plane and C-Plane closed\n");
        goto nlmsg_failure;
    }
    if( result < 0 )
    {
            printk(KERN_CRIT"Hello :: netlink_unicast fails\n");
            failed_to_send_hello = 1;
            skb = NULL;
            goto nlmsg_failure;
    }
    return result;

nlmsg_failure: /* Required by NLMSG_PUT */
    printk(KERN_CRIT"Hello :: netlink_unicast fails return code = %d\n",result);
    if (skb != NULL){
        kfree_skb(skb);
        skb = NULL;
    }
    return result;
}
static int send_bye_response( void *msg, int size )
{
    int result;
    struct sk_buff *skb = NULL;
    struct nlmsghdr *nlh = NULL;
    if (0 == usr_pid_bye){
        //printk(KERN_CRIT"User spce Application is not ready\n");
        return -1;
    }
    skb = alloc_skb( NLMSG_SPACE( size ), GFP_ATOMIC );
    if( !skb )
    {
        printk(KERN_CRIT"send_bye_response :: Failed to allocate memory\n");
        return -ENOMEM;
    }
    result = -EINVAL;
    nlh = NLMSG_PUT( skb, 0, 0, 0, size );
    memset(nlh, 0, size);
    memcpy( NLMSG_DATA( nlh ), msg, size );
    if (failed_to_send_bye)
        goto nlmsg_failure;
    if ((bye_sock_z != NULL) && (usr_pid_bye != 0)){
        //Sudhansu :: on failure netlink_unicast will free the buffer (skb)
        result = netlink_unicast( bye_sock_z, skb, usr_pid_bye, 0 );
    }
    else{
        printk(KERN_CRIT"Hello >> Comunication between U-Plane and C-Plane closed\n");
        goto nlmsg_failure;
    }
    if( result < 0 )
    {
            printk(KERN_CRIT"Hello :: netlink_unicast fails\n");
            failed_to_send_bye = 1;
            skb = NULL;
            goto nlmsg_failure;
    }
    return result;

nlmsg_failure: /* Required by NLMSG_PUT */
    printk(KERN_CRIT"Hello :: netlink_unicast fails return code = %d\n",result);
    if (skb != NULL){
        kfree_skb(skb);
        skb = NULL;
    }
    return result;
}
int process_hello_commands(char *msg, int len)
{
    int iRet = -1;

    spin_lock(&send_hello_lock);
    iRet = send_hello_response(msg, len);
    spin_unlock(&send_hello_lock);
}
int process_bye_commands(char *msg, int len)
{
    int iRet = -1;

    spin_lock(&send_bye_lock);
    iRet = send_bye_response(msg, len);
    spin_unlock(&send_bye_lock);
}
static int process_hello_requests(void *data)
{
    int result;
    struct sk_buff *skb = NULL;
    struct nlmsghdr *nlh = NULL;
    char msg[BUFSIZE];
    
    DECLARE_WAITQUEUE(wait, current);
    daemonize("Hello_Server_z");
    allow_signal(SIGKILL);
    
    set_current_state( TASK_INTERRUPTIBLE );
    add_wait_queue(&event_wait_hello_z, &wait);

    if( !skb_queue_len( &hello_sock_z->sk_receive_queue ) )
    {
        schedule();
    }
    set_current_state(TASK_RUNNING);
    remove_wait_queue(&event_wait_hello_z, &wait);

    skb = skb_dequeue(&hello_sock_z->sk_receive_queue );
    if( skb )
    {
        nlh = (struct nlmsghdr *)skb->data;
        usr_pid_hello = nlh->nlmsg_pid;
        
        printk(KERN_CRIT"Hello Server, user pid = %d\n",usr_pid_hello);
        printk(KERN_CRIT"Data from Hello Client = %s\n",(char *)NLMSG_DATA(nlh));
        
        memset(msg, 0, BUFSIZE);
        memcpy( msg, NLMSG_DATA(nlh), BUFSIZE );
        strncpy( msg, "Hello Server Good day Client", BUFSIZE);
        
        process_hello_commands(msg, BUFSIZE);
        
        if (skb != NULL)
            kfree_skb( skb );
        
        result = 0;
    }
    else
    {
        result = -1;
    }
    return result;
}
static int process_bye_requests(void *data)
{
    int result;
    struct sk_buff *skb = NULL;
    struct nlmsghdr *nlh = NULL;
    char msg[BUFSIZE];
    
    DECLARE_WAITQUEUE(wait, current);
    daemonize("Bye_Server_z");
    allow_signal(SIGKILL);
    
    set_current_state( TASK_INTERRUPTIBLE );
    add_wait_queue(&event_wait_bye_z, &wait);

    if( !skb_queue_len( &bye_sock_z->sk_receive_queue ) )
    {
        schedule();
    }
    set_current_state(TASK_RUNNING);
    remove_wait_queue(&event_wait_bye_z, &wait);

    skb = skb_dequeue(&bye_sock_z->sk_receive_queue );
    if( skb )
    {
        nlh = (struct nlmsghdr *)skb->data;
        usr_pid_bye = nlh->nlmsg_pid;
        
        printk(KERN_CRIT"Bye Server, user pid = %d\n",usr_pid_bye);
        printk(KERN_CRIT"Bye Client message  = %s\n",(char *)NLMSG_DATA(nlh));
    
        memset(msg, 0, BUFSIZE);
        memcpy( msg, NLMSG_DATA(nlh), BUFSIZE );
        strncpy( msg, "BYE Server Good Evening Client", BUFSIZE);
        
        process_bye_commands(msg, BUFSIZE);
        
        if (skb != NULL)
            kfree_skb( skb );
        
        result = 0;
    }
    else
    {
        result = -1;
    }
    return result;
}
int createThread_hello()
{
    hello_thread_id = kernel_thread(process_hello_requests, NULL, CLONE_FS | CLONE_FILES | CLONE_SIGHAND | SIGCHLD | CLONE_KERNEL);
    if (hello_thread_id == 0)
        return -EIO;
    return hello_thread_id;
}
int createThread_bye()
{
    bye_thread_id = kernel_thread(process_bye_requests, NULL, CLONE_FS | CLONE_FILES | CLONE_SIGHAND | SIGCHLD | CLONE_KERNEL);
    if (bye_thread_id == 0)
        return -EIO;
    return bye_thread_id;
}
static void Uplane_receive_hello( struct sock *sk, int len )
{
    wake_up_interruptible( &event_wait_hello_z);
}
static void Uplane_receive_bye( struct sock *sk, int len )
{
    wake_up_interruptible( &event_wait_bye_z);
}

int init_module(void)
{
    //Initializing spin_lock
    spin_lock_init(&send_hello_lock);
    spin_lock_init(&send_bye_lock);
    
    
    printk(KERN_CRIT"********* Creating Hello Thread *********\n");
    hello_sock_z = netlink_kernel_create( NETLINK_HELLO, Uplane_receive_hello);
    if (NULL == hello_sock_z)
    {
        printk(KERN_CRIT"Failed to create Hello Netlink Socket .. Exiting \n");
        return -1;
    }
    hello_thread_id = createThread_hello();
    if (-EIO == hello_thread_id)
    {
        printk(KERN_CRIT"Failed to create Hello Thread, return %d\n",hello_thread_id);
        sock_release( hello_sock_z->sk_socket );
        return -1;
    }
    printk(KERN_CRIT"Hello Thread ID = %d\n",hello_thread_id);

    printk(KERN_CRIT"********* Creating Bye Thread *********\n");
    bye_sock_z = netlink_kernel_create( NETLINK_BYE, Uplane_receive_bye);
    if (NULL == bye_sock_z)
    {
        printk(KERN_CRIT"Failed to create Bye Netlink Socket .. Exiting \n");
        cleanup_module();
        return -1;
    }
    bye_thread_id = createThread_bye();
    if (-EIO == bye_thread_id)
    {
        printk(KERN_CRIT"Failed to create Hello Thread, return %d\n", bye_thread_id);
        cleanup_module();
        return -1;
    }
    printk(KERN_CRIT"Bye Thread ID = %d\n",bye_thread_id);

    register_IPv4_in_hook();

    return 0;
    
}
void cleanup_module(void)
{
    if (hello_thread_id != 0)
    {
        kill_proc(hello_thread_id, SIGKILL, 1);
        msleep(10L);
        printk(KERN_CRIT"Thread Id [%d] killed ....",hello_thread_id);
        hello_thread_id = 0;
    }
    if (bye_thread_id != 0)
    {
        kill_proc(bye_thread_id, SIGKILL, 1);
        msleep(10L);
        printk(KERN_CRIT"Thread Id [%d] killed ....",bye_thread_id);
        bye_thread_id = 0;
    }
    if (hello_sock_z)
    {
        sock_release( hello_sock_z->sk_socket );
        hello_sock_z = NULL;
    }
    if (bye_sock_z)
    {
        sock_release( bye_sock_z->sk_socket );
        bye_sock_z = NULL;
    }
    if (nf_registered_hello_ipv4)
    {
        nf_unregister_hook(&hello_IPv4_inHook_ops);
        nf_registered_hello_ipv4 = 0;
    }
    if (nf_registered_bye_ipv4)
    {
        nf_unregister_hook(&bye_IPv4_inHook_ops);
        nf_registered_bye_ipv4 = 0;
    }
}
void register_IPv4_in_hook()
{
    if (!nf_registered_hello_ipv4)
    {
        nf_register_hook(&hello_IPv4_inHook_ops);
        nf_registered_hello_ipv4 = 1;
    }
    if (!nf_registered_bye_ipv4)
    {
        nf_register_hook(&bye_IPv4_inHook_ops);
        nf_registered_bye_ipv4 = 1;
    }
}
MODULE_AUTHOR("Sudhansu Mishra");
MODULE_LICENSE("GPL");
