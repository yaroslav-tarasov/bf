#include <QDebug>
#include <QTimer>
#include <QStringList>
#include <QFile>
#include <QSharedPointer>

#include "cmds.h"

namespace cmds
{
const int fieldWidth = 7;
int fieldWidthSaved;

template<typename T>
void  inline printMessage( QTextStream &out, T msg, filter_rule_t fr)
{
    out << msg << endl;
    printHeader2(out);
    out << qSetFieldWidth(fieldWidth) << fr;
    out << qSetFieldWidth(fieldWidthSaved) << endl;
}

void addRule(const filter_rule_t& fr, QTextStream& qout, BFControlBase& bfc)
{
    int ret = bfc.addRule(fr);
    if(ret>=0){
        printMessage(qout,"Add new rule:",fr);
    }
    else if(ret==-BF_ERR_ALREADY_HAVE_RULE)
    {
        printMessage(qout,"Already have this rule:",fr);
    }
}

void listRules(const filter_rule_t& fr, QTextStream& qout, BFControlBase& bfc)
{
    QList<filter_rule_ptr > ruleslst;
    int ret =  bfc.getRulesSync(fr,  ruleslst);
    if(ret>=0)
    {
        int i;
        qout.setFieldAlignment(QTextStream::AlignCenter);

        // qout.setPadChar('-');

        bf_chain_t chains[2] = {CHAIN_INPUT,CHAIN_OUTPUT};
        bf_policy_t policies[2] = {POLICY_NONE,POLICY_NONE};

        foreach (filter_rule_ptr rule,ruleslst){
            filter_rule_t fr_out = *static_cast<filter_rule_t*>(rule.data());
            if (policy_rule(&fr_out.base))
            {
               if(fr_out.base.chain == CHAIN_INPUT)
                   policies[0] = static_cast<bf_policy_t>(fr_out.policy);
               else if(fr_out.base.chain == CHAIN_OUTPUT)
                   policies[1] = static_cast<bf_policy_t>(fr_out.policy);

            }
        }

        const int ch_num = sizeof(chains)/sizeof(chains[0]);
        for(int ch = 0; ch < ch_num; ++ch )
        {
            if( fr.base.chain==chains[ch] || fr.base.chain==CHAIN_ALL)
            {
                qout << "Chain " << get_chain_name(chains[ch])  << " (policy "<< get_policy_name(policies[ch]) << ")" << qSetFieldWidth(fieldWidthSaved) << endl;
                printHeader(qout);
            }

            i = 0;
            qout << qSetFieldWidth(fieldWidthSaved) << endl;
        foreach (filter_rule_ptr rule,ruleslst){
            filter_rule_t fr_out = *static_cast<filter_rule_t*>(rule.data());
            if (policy_rule(&fr_out.base))
            {
               continue;
            }

            {
                if (fr_out.base.chain==chains[ch] && (fr.base.chain==chains[ch] || fr.base.chain==CHAIN_ALL ) )
                {
                    qout  << qSetFieldWidth(fieldWidth) << i++ << fr_out;
                    qout << qSetFieldWidth(fieldWidthSaved) << endl;
                }
            }
        }
        qout << qSetFieldWidth(fieldWidthSaved) << endl;
        }
        qout << qSetFieldWidth(fieldWidthSaved) << endl;
    }
}

void deleteRule(const filter_rule_t& fr, QTextStream& qout, BFControlBase& bfc)
{
    int ret = bfc.deleteRule(fr);
    if(ret>=0){
        printMessage(qout,"Rule deleted:",fr);
    }
    else if(ret==-BF_ERR_MISSING_RULE)
    {
        printMessage(qout,"Do not have this rule:",fr);
    }
}

void updateRule(const filter_rule_t& fr, QTextStream& qout, BFControlBase &bfc)
{
    int ret = bfc.updateRule(fr);
    if(ret>=0)
    {
        printMessage(qout,"Rule updated:",fr);
    }
    else if(ret==-BF_ERR_MISSING_RULE)
    {
        printMessage(qout,"Do not have this rule:",fr);
    }
}

void setChainPolicy(const filter_rule_t& fr, QTextStream& qout, BFControlBase& bfc)
{
    int ret = bfc.setChainPolicy(fr);
    if(ret>=0)
    {
        qout <<"Set policy for chain (" << get_chain_name(static_cast<bf_chain_t>(fr.base.chain))<< "): " << get_policy_name(static_cast<bf_policy_t>(fr.policy));
    }
    else if(ret==-BF_ERR_SOCK)
    {
        qout <<"Can not set policy for chain (" << get_chain_name(static_cast<bf_chain_t>(fr.base.chain))<< "): " << get_policy_name(static_cast<bf_policy_t>(fr.policy));
    }
}

int loadFromFile(const std::string& thename, BFControlBase& bfc)
{
    QList<filter_rule_ptr > ruleslst;
    //qDebug() << "CMD_LOAD_FROM_FILE";
    QFile  file(QString::fromStdString(thename)) ;
    if (!file.open(QIODevice::ReadOnly))  return -1;
    QDataStream in(&file);

    while(!in.atEnd()){
        filter_rule_t fr;
        in >>  fr;
        ruleslst.append(filter_rule_ptr(new filter_rule_t(fr)));;
    }
    file.close();

    if(ruleslst.size()>0)
        bfc.sendRulesSync(ruleslst);
    return 0;
}

}
