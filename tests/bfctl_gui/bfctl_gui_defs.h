#ifndef BFCTL_GUI_DEFS_H
#define BFCTL_GUI_DEFS_H

namespace bfmodel {
  enum     e_roles_t { DirtyRole=(Qt::UserRole + 1), IdRole};
  enum     e_index_t {SRCIP,SRCPORT,DSTIP,DSTPORT,PROTO,CHAIN,POLICY,OFF};
}

#endif // BFCTL_GUI_DEFS_H
