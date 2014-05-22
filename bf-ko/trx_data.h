#pragma once

#if defined(__cplusplus) && defined(QT_VERSION)
#include <QtGlobal>
#include <QDataStream>
#include <QDebug>
#include <QHostAddress>
#include <arpa/inet.h>
#endif

#include <linux/if_ether.h>
#include <linux/netlink.h>

/**
@brief
    Цепочки правил

*/
// Для фильтра только CHAIN_INPUT,CHAIN_OUTPUT остальные значения не корректны для м.я.
enum bf_chain_t {
             CHAIN_NONE,                        //!< Отсутствие наличия цепи
             CHAIN_ALL,                         //!< все цепи сразу
             CHAIN_INPUT,                       //!< цепь входящих
             CHAIN_OUTPUT                       //!< цепь исходящих
};
/**
@brief
    Политики фильтрации пакетов

*/
enum bf_policy_t {
             POLICY_NONE,                       //!< Отсутствие наличия политики
             POLICY_DROP,                       //!< Отбрасываем пакет
             POLICY_ACCEPT                      //!< Пропускаем пакет дальше
};

/**
@brief
    Включение отключение правил

*/
enum bf_switch_rules_t {                  //!<  Для поля off YES - выключено NO - включено
             SW_NO,                       //!<  Правило включено
             SW_YES,                      //!<  Правило отключено
             SW_NONE                      //!<  Отсутствие наличия правила выключения
};

/**
@brief
    Ошибки

*/
enum bf_error_t {
            BF_ERR_OK,                    //!< Успешная опереация
            BF_ERR_ALREADY_HAVE_RULE,     //!< Правило есть в наличии
            BF_ERR_MISSING_RULE,          //!< Операция применяется к отсутствующему правилу
            BF_ERR_SOCK                   //!< Ошибка сокета
};


enum {IPPROTO_NOTEXIST=65000,IPPROTO_ALL};      // Fake proto

static inline const char* get_sw_name(enum bf_switch_rules_t s) {
    static const char* sw_names[]=
    {"NO","YES","NONE"};
    if ( SW_NO == s) {
        return sw_names[0];
    } else if (SW_YES == s ) {
        return sw_names[1];
    }

    return sw_names[2];
}

static inline const char* get_policy_name(enum bf_policy_t p) {
    static const char* p_names[]=
    {"DROP","ACCEPT","NONE"};
    if ( POLICY_DROP == p) {
        return p_names[0];
    } else if (POLICY_ACCEPT == p ) {
        return p_names[1];
    }

    return p_names[2];
}

static inline const char* get_proto_name(int proto) {
    static const char* proto_names[]=
    {"ALL","TCP","UDP","NONE"};
    if (IPPROTO_NOTEXIST == proto) {
        return proto_names[0];
    } else if (IPPROTO_TCP == proto ) {
        return proto_names[1];
    } else if (IPPROTO_UDP == proto ) {
        return proto_names[2];
    }
    return proto_names[3];
}

static inline const char* get_chain_name(enum bf_chain_t chain) {
    static const char* chain_names[]=
    {"ALL","INPUT","OUTPUT","NONE"};
    if ( CHAIN_ALL== chain) {
        return chain_names[0];
    } else if (CHAIN_INPUT == chain ) {
        return chain_names[1];
    } else if (CHAIN_OUTPUT == chain ) {
        return chain_names[2];
    }
    return chain_names[3];
}

/**
@brief
    Сообщения для обмена с ko

*/

enum bf_messages_t {
                  MSG_ADD_RULE=NLMSG_MIN_TYPE + 2, //!< Добавление правила
                  MSG_DATA,                     //!< При пересылке данных из модуля ядра в  userspace
                  MSG_DONE,                     //!< По окончании пересылки данных из ядра
                  MSG_DELETE_RULE ,             //!< Удаление конкретного правила
                  MSG_DELETE_ALL_RULES,         //!< Удаление всех правил (не реализовано)
                  MSG_UPDATE_RULE,              //!< (не реализовано)
                  MSG_CHAIN_POLICY,             //!< Конечное правило для цепочки
                  MSG_GET_RULES,                //!< Получение правил из модуля ядра
                  MSG_OK,                       //!< Подтверждние
                  MSG_ERR,                      //!< Ошибка
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
} log_subscribe_msg_t;

typedef struct msg_done
{
    u_int32_t counter;
} msg_done_t;

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
    __u8  chain;
    __u16 proto;
    __u16 src_port;
    __u16 dst_port;
    ip_addr_t s_addr;
    ip_addr_t d_addr;
#ifdef __cplusplus
    explicit  filter_rule_base( __u16 proto=0, __u16 src_port=0, __u16 dst_port=0,__u32   s_addr=0,__u32   d_addr=0,__u8 chain=0)
        :chain(chain),proto(proto),src_port(src_port),dst_port(dst_port),s_addr(s_addr),d_addr(d_addr){}

    bool operator==(const filter_rule_base& s) const
    {
        return (proto == s.proto &&
                src_port == s.src_port &&
                dst_port == s.dst_port &&
                s_addr.addr == s.s_addr.addr &&
                d_addr.addr == s.d_addr.addr);
    }
#endif
} filter_rule_base_t;

typedef struct filter_rule{
    unsigned char	h_dest[ETH_ALEN];
    unsigned char	h_source[ETH_ALEN];
   filter_rule_base_t base;
   __u8  off;	    
   __u8  policy; 
   __u32 id;
#ifdef __cplusplus
   explicit  filter_rule(__u16 proto=0, __u16 src_port=0, __u16 dst_port=0,__u8  chain=0,__u8  policy=0)
       :base(proto,src_port,dst_port,0,0,chain),off(0),policy(policy){}
#endif
} filter_rule_t;

typedef struct msg_err
{
    filter_rule_t fr;
    u_int16_t code;
} msg_err_t;

#pragma pack ()

#if !defined (__cplusplus) && !defined (__KERNEL__)
   typedef int bool;
   #define true   (1)
   #define false  (0)
#endif

static inline bool fr_pattern(const filter_rule_t *fr1, const filter_rule_t *fr2)
{
    return  (fr2->base.chain==0?true:(fr1->base.chain==fr2->base.chain)) &&
            (fr2->base.dst_port==0?true:(fr1->base.dst_port==fr2->base.dst_port)) &&
            (fr2->base.d_addr.addr==0?true:(fr1->base.d_addr.addr==fr2->base.d_addr.addr)) &&
            (fr2->base.proto==0?true:(fr1->base.proto==fr2->base.proto)) &&
            (fr2->base.src_port==0?true:(fr1->base.src_port==fr2->base.src_port)) &&
            (fr2->base.s_addr.addr==0?true:(fr1->base.s_addr.addr==fr2->base.s_addr.addr)) &&
            (fr2->off==0?true:(fr1->off==fr2->off)) &&
            (fr2->policy==0?true:(fr1->policy==fr2->policy));
}

#if defined (__cplusplus) && defined(QT_VERSION)
inline QDataStream &operator <<(QDataStream &stream,const filter_rule_t &fr)
{
    stream << fr.base.src_port;
    stream << fr.base.dst_port;
    stream << fr.base.s_addr.addr;
    stream << fr.base.d_addr.addr;
    stream << fr.base.proto;
    stream << fr.base.chain;
    stream << fr.policy;
    stream << fr.off;

    return stream;
}

inline QDataStream &operator >>(QDataStream &stream, filter_rule_t &fr)
{
    stream >> fr.base.src_port;
    stream >> fr.base.dst_port;
    stream >> fr.base.s_addr.addr;
    stream >> fr.base.d_addr.addr;
    stream >> fr.base.proto;
    stream >> fr.base.chain;
    stream >> fr.policy;
    stream >> fr.off;

    return stream;
}

inline QDebug operator<<(QDebug dbg, const filter_rule_t &fr)
{
    dbg.space() <<  "src_port:" << fr.base.src_port
                  << "dst_port:" << fr.base.dst_port
                  << "s_addr.addr:" << QHostAddress(static_cast<quint32>(htonl(fr.base.s_addr.addr))).toString()
                  << "d_addr.addr:" << QHostAddress(static_cast<quint32>(htonl(fr.base.d_addr.addr))).toString()
                  << "proto:" << get_proto_name(fr.base.proto)
                  << "chain:" << get_chain_name(static_cast<bf_chain_t>(fr.base.chain))
                  << "policy:" << get_policy_name(static_cast<bf_policy_t>(fr.policy))
                  << "off:" << get_sw_name(static_cast<bf_switch_rules_t>(fr.off));
    return dbg.space();
}

inline QTextStream& operator<<(QTextStream &out, const filter_rule_t &fr)
{
       return out << QHostAddress(static_cast<quint32>(htonl(fr.base.s_addr.addr))).toString()
                  << fr.base.src_port
                  << QHostAddress(static_cast<quint32>(htonl(fr.base.d_addr.addr))).toString()
                  << fr.base.dst_port
                  << get_proto_name(fr.base.proto)
                  << get_chain_name(static_cast<bf_chain_t>(fr.base.chain))
                  << get_policy_name(static_cast<bf_policy_t>(fr.policy))
                  << get_sw_name(static_cast<bf_switch_rules_t>(fr.off)) ;
}

inline void printHeader(QTextStream &out)
{
    int width = out.fieldWidth();
    out << qSetFieldWidth(10) << "rule #" << "s_addr" <<  "src_port"
         << "d_addr" << "dst_port"
         << "proto"
         << "chain"
         << "policy"
         << "off" << qSetFieldWidth(width) << endl ;

}

inline void printHeader2(QTextStream &out)
{
    int width = out.fieldWidth();
    out << qSetFieldWidth(10) << "s_addr" <<  "src_port"
         << "d_addr" << "dst_port"
         << "proto"
         << "chain"
         << "policy"
         << "off" << qSetFieldWidth(width) << endl ;
}

#ifdef  BOOST_REFLECTION

REFL_STRUCT(ip_addr_t)
    REFL_ENTRY(addr)
REFL_END()

REFL_STRUCT(filter_rule_base_t)
    REFL_ENTRY(chain)
    REFL_ENTRY(proto)
    REFL_ENTRY(src_port)
    REFL_ENTRY(dst_port)
    REFL_ENTRY(s_addr)
    REFL_ENTRY(d_addr)
REFL_END()

REFL_STRUCT(filter_rule_t)
    REFL_ENTRY(base)
    REFL_ENTRY(off)
    REFL_ENTRY(policy)
    REFL_ENTRY(id)
REFL_END()

typedef vector<filter_rule_t> rules_t;

typedef struct config
{
    vector<uint8_t> chain_policy;
    rules_t         rules;
    config():chain_policy(2){};
} config_t;

REFL_STRUCT(config_t)
    REFL_ENTRY(chain_policy)
    REFL_ENTRY(rules)
REFL_END()


#endif

#endif



