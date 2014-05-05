#ifndef BF_CONFIG_H
#define BF_CONFIG_H

/**
 * @brief: Main configuration structure
 * @member: netfilter_ops      Netfilter operations
 * @member: list               Lists containing the filtered data
 * @member: skb_list           Buffer list used between the pattern matching
 *                             and the packet enqueueing
 * @member: work_queue         Work queue used to (en/de)queue packets
 */

struct  nf_bf_filter_config {
    struct nf_hook_ops      nfho_in;   //net filter hook option struct for input
    struct nf_hook_ops      nfho_out;  //net filter hook option struct for output
    struct work_struct      work_logging;
    struct workqueue_struct *wq_logging;
    struct sk_buff_head     *skb_list;
    
    uint8_t  chain_rule[2]; // Конечное правило для цепочек INPUT OUTPUT, по умолчанию ACCEPT,
                            // цепочек фиксированное колличество, 2 штуки на входе и на выходе      
    atomic_t init;
    pid_t    pid_log;
};

typedef struct filter_rule_list {
    filter_rule_t fr;
    struct list_head full_list; /* kernel's list structure */
    struct list_head chain_list; /* kernel's list structure */
    struct hash_entry entry;
    struct rcu_head rcu;
} filter_rule_list_t;


struct sock * get_nl_sock(void);
void  wfc(void);
pid_t get_client_pid(void);
int   nl_send_msg(struct sock * nl_sk,int destpid, int type,int flags,char* msg,int msg_size);
int   nl_send_lst(struct sock * nl_sk,int destpid,  filter_rule_list_t* lst,int lst_size,int* end_list);


int   find_rule(unsigned char* data);
void  add_rule(struct filter_rule* fr);
void  delete_rule(struct filter_rule* fr);
int   nl_send_msg(struct sock * nl_sk,int destpid, int type, int flags,char* msg,int msg_size);
void  list_rules(struct sock * nl_sk,int destpid);
void  delete_rules(void);


#endif // BF_CONFIG_H
