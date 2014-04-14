//'Hello World' netfilter hooks example
//For any packet, we drop it, and log fact to /var/log/messages

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/udp.h>
#include <linux/ip.h>

static struct nf_hook_ops nfho;         //struct holding set of hook function options
struct sk_buff *sock_buff;
struct iphdr *ip_header;
struct udphdr *udp_header;
typedef struct iphdr iphdr_t; 
typedef struct udphdr udphdr_t;

//function to be called by hook
unsigned int hook_func(unsigned int hooknum, struct sk_buff *skb, const struct net_device *in, const struct net_device *out, int (*okfn)(struct sk_buff *))
{
  sock_buff = skb;
  
  ip_header = (struct iphdr *)skb_network_header(sock_buff);   // grab networkheader
	
  if(!sock_buff) {return NF_ACCEPT;}

  if(ip_header->protocol==1){
	udp_header = (struct udphdr *) skb_transport_header(sock_buff);
	
	// printk
	return NF_DROP;
  }
	


  // printk(KERN_INFO "packet dropped\n");                                             //log to var/log/messages
  return NF_ACCEPT;                                                                   //drops the packet
}

//Called when module loaded using 'insmod'
int init_module()
{
  nfho.hook = hook_func;                       //function to call when conditions below met
  nfho.hooknum = NF_INET_PRE_ROUTING;            //called right after packet recieved, first hook in Netfilter
  nfho.pf = PF_INET;                           //IPV4 packets
  nfho.priority = NF_IP_PRI_FIRST;             //set to highest priority over all other hook functions
  nf_register_hook(&nfho);                     //register hook

  return 0;                                    //return 0 for success
}

//Called when module unloaded using 'rmmod'
void cleanup_module()
{
  nf_unregister_hook(&nfho);                     //cleanup – unregister hook
}

MODULE_AUTHOR("Erik Schweigert");
MODULE_DESCRIPTION("SK Buff Parse Module");
MODULE_LICENSE("GPL");
