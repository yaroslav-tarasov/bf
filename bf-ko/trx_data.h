#pragma once

#ifdef __cplusplus
#include <QtGlobal>
#include <QDataStream>
#endif

// Направление для фильтра только DIR_INPUT,DIR_OUTPUT остальные значения не корректны для м.я.
enum { DIR_NONE,DIR_ALL,DIR_INPUT,DIR_OUTPUT};
enum {POLICY_NONE,POLICY_DROP,POLICY_ACCEPT};
enum {IPPROTO_NOTEXIST=65000,IPPROTO_ALL};
enum bf_messages {MSG_ADD_RULE=NLMSG_MIN_TYPE + 2,MSG_DATA,MSG_DONE,MSG_RULE_ERR,MSG_DELETE_RULE ,MSG_DELETE_ALL_RULES,MSG_UPDATE_RULE,MSG_GET_RULES,MSG_OK};

#pragma pack (1)
typedef struct _ip_addr
{
   __u32   addr;  // WARNING ipv6 must be 128 bit
#ifdef __cplusplus
   explicit _ip_addr(__u32   addr=0):addr(addr){}
#endif
} ip_addr_t;

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
   __u8  direction;
   __u8  policy; 
   __u32 id;
#ifdef __cplusplus
   explicit  filter_rule(__u16 proto=0, __u16 src_port=0, __u16 dst_port=0,__u8  direction=DIR_INPUT,__u8  policy=POLICY_ACCEPT)
       :base_rule(proto,src_port,dst_port),direction(direction),policy(policy){off=0;}
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
    stream << fr.direction;
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
    stream >> fr.direction;
    stream >> fr.policy;
    stream >> fr.off;

    return stream;
}
#endif



