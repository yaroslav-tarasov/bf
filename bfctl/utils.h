#ifndef UTILS_H
#define UTILS_H

namespace cmd_utils {
int
get_proto(char* proto);

inline 	const char*
get_proto_name(int proto) {
    static const char* proto_names[]=
    {"ALL","TCP","UDP"};
    if (IPPROTO_NOTEXIST == proto) {
        return proto_names[0];
    } else if (IPPROTO_TCP == proto ) {
        return proto_names[1];
    } else if (IPPROTO_UDP == proto ) {
        return proto_names[2];
    }
    return NULL;
}

int
get_direction(char* dir);


int
get_policy(char* policy);

int
parse_cmd_args(int argc, char *argv[],filter_rule_t* td);

}

enum { CMD_NEW_RULE=1,CMD_PRINT_RULES,CMD_DEL_RULE,CMD_PRINT_HELP};
enum { DIR_ALL,DIR_INPUT,DIR_OUTPUT};

#endif // UTILS_H
