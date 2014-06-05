#include <linux/if_ether.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <linux/netlink.h>

#include "trx_data.h"
#include "bfcontrolbase.h"

namespace cmds
{

void addRule        (const filter_rule_t& fr, QTextStream& qout, BFControlBase& bfc);
void listRules      (const filter_rule_t& fr, QTextStream& qout, BFControlBase& bfc);
void deleteRule     (const filter_rule_t& fr, QTextStream& qout, BFControlBase& bfc);
void updateRule     (const filter_rule_t& fr, QTextStream& qout, BFControlBase& bfc);
void setChainPolicy (const filter_rule_t& fr, QTextStream& qout, BFControlBase& bfc);
int  loadFromFile   (const std::string& thename, BFControlBase &bfc);

}
