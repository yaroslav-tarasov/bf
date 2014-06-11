#ifndef BFCTL_GUI_DEFS_H
#define BFCTL_GUI_DEFS_H

namespace bfmodel {
  static   const int        DirtyRole=(Qt::UserRole + 1);
  enum     e_index_t {SRCIP,SRCPORT,DSTIP,DSTPORT,PROTO,CHAIN,POLICY,OFF};
}

#endif // BFCTL_GUI_DEFS_H
