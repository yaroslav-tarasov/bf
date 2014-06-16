#ifndef UTILS_H
#define UTILS_H

#include <string>

namespace cmd_utils {

void exit_printhelp();


struct cmd_args
{
    filter_rule_t fr;
    std::string   file_name;
    cmd_args(const filter_rule_t& fr,const std::string& file_name):fr(fr),file_name(file_name){}
};

int parse_cmd_args(int argc, char *argv[], cmd_args& ca );

}

enum commands_t {
       CMD_NONE,
       CMD_INSERT,
       CMD_DELETE =         0x0002U,
       //CMD_DELETE_NUM =   0x0004U,
       //CMD_REPLACE=		0x0008U,
       CMD_APPEND=		    0x0010U,
       CMD_LIST=		    0x0020U,
       CMD_FLUSH=		    0x0040U,
       //CMD_ZERO=		    0x0080U,
       //CMD_NEW_CHAIN=		0x0100U,
       //CMD_DELETE_CHAIN=	0x0200U,
       CMD_SET_POLICY=		0x0400U,
       //CMD_RENAME_CHAIN=	0x0800U,
       //CMD_LIST_RULES=	0x1000U,
       //CMD_ZERO_NUM=		0x2000U,
       //CMD_CHECK=		    0x4000U,
       CMD_UPDATE=          0x8000U,
       CMD_LOAD_FROM_FILE=  0x10000U,
       CMD_PRINT_HELP=     0x20000U
};


#endif // UTILS_H
