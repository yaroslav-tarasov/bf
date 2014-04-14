#pragma once

#pragma pack (1)
typedef struct _ip_addr
{
   __u32   addr;  // WARNING ipv6 must be 128 bit
#ifdef __cplusplus
   explicit _ip_addr(__u32   addr=0):addr(addr){};
#endif
} ip_addr_t;

typedef struct filter_rule_base {
    ip_addr_t d_addr;
    ip_addr_t s_addr;

    __u16 proto;
    __u16 src_port;
    __u16 dst_port;
#ifdef __cplusplus
    explicit  filter_rule_base( __u16 proto=0, __u16 src_port=0, __u16 dst_port=0):proto(proto),src_port(src_port),dst_port(dst_port){}
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
   filter_rule(__u16 proto=0, __u16 src_port=0, __u16 dst_port=0):base_rule(proto,src_port,dst_port){};
#endif
} filter_rule_t;

#pragma pack ()

enum {POLICY_DROP,POLICY_ACCEPT};
enum {IPPROTO_NOTEXIST=65000};
enum {MSG_ADD_RULE=NLMSG_MIN_TYPE + 2,MSG_DATA,MSG_DONE,MSG_RULE_ERR,MSG_DELETE_RULE ,MSG_DELETE_ALL_RULES,MSG_UPDATE_RULE,MSG_GET_RULES,MSG_OK};

