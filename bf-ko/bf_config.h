#ifndef BF_CONFIG_H
#define BF_CONFIG_H

enum chain_p_t {INPUT,OUTPUT};

/**
 * @brief: Main configuration structure
 * @member: nfho_in            Netfilter input hook
 * @member: nfho_out           Netfilter output hook
 * @member: skb_list           Buffer list used between the pattern matching
 *                             and the packet enqueueing
 * @member: wq_logging         Work queue used to logging
 */

struct  nf_bf_filter_config {
    struct nf_hook_ops      nfho_in;   //!< net filter hook option struct for input
    struct nf_hook_ops      nfho_out;  //!< net filter hook option struct for output
    struct work_struct      work_logging; //!< Задача логгирования
    struct workqueue_struct *wq_logging;  //!< Очередь для лога
    struct sk_buff_head     *skb_list;    //!< Список отфильтрованных пакетов для лога
    
    uint8_t  chain_policy[2]; //!< Конечное правило для цепочек INPUT OUTPUT, по умолчанию ACCEPT,
                              //!< цепочек фиксированное колличество, 2 штуки, одна на входе и одна на выходе
    atomic_t init;
    pid_t    pid_log;         //!< Pid процесса подписчика на лог. Только один подписчик.

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


int   find_rule(unsigned char* data, struct filter_rule_list **res);
void  add_rule(struct filter_rule* fr);
void  delete_rule(struct filter_rule* fr);
int   nl_send_msg(struct sock * nl_sk,int destpid, int type, int flags,char* msg,int msg_size);
void  list_rules(struct sock * nl_sk, int destpid, filter_rule_t* pfr_pattern);
void  delete_rules(void);

enum bf_policy_t convert_policy(int p);
int  apply_policy(enum bf_policy_t p);

extern int fdebug;

inline static void __printfr(const char* func ,filter_rule_t fr)
{
    printk(KERN_INFO "%s: SRC: (%u.%u.%u.%u):%d --> DST: (%u.%u.%u.%u):%d proto: %d; \n", func,
                    NIPQUAD(fr.base.s_addr.addr),fr.base.src_port,
                    NIPQUAD(fr.base.d_addr.addr),fr.base.dst_port, fr.base.proto);
}

#define  PRINTFR(fr)  __printfr(__FUNCTION__ , fr);
#define  PRINTK_DBG  if(fdebug) printk

#endif // BF_CONFIG_H
