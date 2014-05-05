#pragma once

#if defined(__cplusplus) && defined(QT_VERSION)
#include <QtGlobal>
#include <QDataStream>
#include <QDebug>
#include <QHostAddress>
#endif

// Для фильтра только CHAIN_INPUT,CHAIN_OUTPUT остальные значения не корректны для м.я.
enum chain { CHAIN_NONE,                        //!< Отсутствие наличия цепи
             CHAIN_ALL,                         //!< все цепи сразу
             CHAIN_INPUT,                       //!< цепь входящих
             CHAIN_OUTPUT                       //!< цепь исходящих
           };
/**
@brief
    Политики фильтрации пакетов

*/
enum policy {
             POLICY_NONE,                       //!< Отсутствие наличия политики
             POLICY_DROP,                       //!< Отбрасываем пакет
             POLICY_ACCEPT                      //!< Пропускаем пакет дальше
            };

enum {IPPROTO_NOTEXIST=65000,IPPROTO_ALL};      // Fake proto

/**
@brief
    Сообщения для обмена с ko

*/

enum bf_messages {
                  MSG_ADD_RULE=NLMSG_MIN_TYPE + 2, //!< Добавление правила
                  MSG_DATA,                     //!< При пересылке данных из модуля ядра в  userspace
                  MSG_DONE,                     //!< По окончании пересылки данных из ядра
                  MSG_RULE_ERR,
                  MSG_DELETE_RULE ,             //!< Удаление конкретного правила
                  MSG_DELETE_ALL_RULES,         //!< Удаление всех правил (не реализовано)
                  MSG_UPDATE_RULE,              //!< (не реализовано)
                  MSG_CHAIN_POLICY,             //!< Конечное правило для цепочки
                  MSG_GET_RULES,                //!< Получение правил из модуля ядра
                  MSG_OK,                       //!< Подтверждние
                  MSG_LOG,                      //!< Лог из модуля ядра
                  MSG_LOG_SUBSCRIBE             //!< Подписка на лог (реализован только один подписчик)
                 };

#pragma pack (1)
/**
@brief
    Подписка на лог из ko

*/
typedef struct _log_subscribe_msg
{
   pid_t   pid;
#ifdef __cplusplus
   explicit _log_subscribe_msg(pid_t   pid=0):pid(pid){}
#endif
} _log_subscribe_msg_t;


typedef struct _ip_addr
{
   __u32   addr;  // WARNING ipv6 must be 128 bit
#ifdef __cplusplus
   explicit _ip_addr(__u32   addr=0):addr(addr){}
#endif
} ip_addr_t;

//  Как вариант базового правила
//  но опять же нужны ли мак адреса? И сетевые маски?
typedef struct {
  u_int8_t smac[ETH_ALEN], dmac[ETH_ALEN]; /* Use '0' (zero-ed MAC address) for any MAC address.
					      This is applied to both source and destination. */
  u_int16_t vlan_id;                   /* Use '0' for any vlan */
  u_int8_t  proto;                     /* Use 0 for 'any' protocol */
  ip_addr_t   shost, dhost;              /* User '0' for any host. This is applied to both source and destination. */
  ip_addr_t   shost_mask, dhost_mask;    /* IPv4/6 network mask */
  u_int16_t sport_low, sport_high;     /* All ports between port_low...port_high means 'any' port */
  u_int16_t dport_low, dport_high;     /* All ports between port_low...port_high means 'any' port */
} filtering_rule_core_fields;

typedef struct filter_rule_base {

	
    __u16 proto;
    __u16 src_port;
    __u16 dst_port;
    ip_addr_t s_addr;
    ip_addr_t d_addr;
#ifdef __cplusplus
    explicit  filter_rule_base( __u16 proto=0, __u16 src_port=0, __u16 dst_port=0,__u32   s_addr=0,__u32   d_addr=0)
        :proto(proto),src_port(src_port),dst_port(dst_port),s_addr(s_addr),d_addr(d_addr){}
#endif
} filter_rule_base_t;

typedef struct filter_rule{
    unsigned char	h_dest[ETH_ALEN];
    unsigned char	h_source[ETH_ALEN];
   filter_rule_base_t base_rule;
   __u8  off;	    
   __u8  chain;
   __u8  policy; 
   __u32 id;
#ifdef __cplusplus
   explicit  filter_rule(__u16 proto=0, __u16 src_port=0, __u16 dst_port=0,__u8  chain=CHAIN_INPUT,__u8  policy=POLICY_ACCEPT)
       :base_rule(proto,src_port,dst_port),off(0),chain(chain),policy(policy){}
#endif
} filter_rule_t;

#pragma pack ()


#if defined (__cplusplus) && defined(QT_VERSION)
inline QDataStream &operator <<(QDataStream &stream,const filter_rule_t &fr)
{
    stream << fr.base_rule.src_port;
    stream << fr.base_rule.dst_port;
    stream << fr.base_rule.s_addr.addr;
    stream << fr.base_rule.d_addr.addr;
    stream << fr.base_rule.proto;
    stream << fr.chain;
    stream << fr.policy;
    stream << fr.off;

    return stream;
}

inline QDataStream &operator >>(QDataStream &stream, filter_rule_t &fr)
{
    stream >> fr.base_rule.src_port;
    stream >> fr.base_rule.dst_port;
    stream >> fr.base_rule.s_addr.addr;
    stream >> fr.base_rule.d_addr.addr;
    stream >> fr.base_rule.proto;
    stream >> fr.chain;
    stream >> fr.policy;
    stream >> fr.off;

    return stream;
}

inline QDebug operator<<(QDebug dbg, const filter_rule_t &fr)
{
    dbg.space() << "filter_rule_t"
                  << "src_port:" << fr.base_rule.src_port
                  << "dst_port:" << fr.base_rule.dst_port
                  << "s_addr.addr:" << QHostAddress(static_cast<quint32>(fr.base_rule.s_addr.addr)).toString()
                  << "d_addr.addr:" << QHostAddress(static_cast<quint32>(fr.base_rule.d_addr.addr)).toString()
                  << "proto:" << fr.base_rule.proto
                  << "chain:" << fr.chain
                  << "policy:" << fr.policy
                  << "off:" << fr.off
                  << "end of filter_rule_t";
    return dbg.space();
}

#endif



