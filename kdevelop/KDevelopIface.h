#ifndef _KDEVELOPIFACE_H_
#define _KDEVELOPIFACE_H_

#include <dcopobject.h>
#include <dcopref.h>

class KDevelopCore;
class KDCOPActionProxy;


class KDevelopIface : public DCOPObject
{
    K_DCOP
    
public:
    KDevelopIface( KDevelopCore *core );
    ~KDevelopIface();

k_dcop:
    bool openProjectSpace(const QString &fileName);
    void closeProjectSpace();
    QCStringList actions();
    DCOPRef action(const QCString &name);

private:
    KDevelopCore *m_core;
    KDCOPActionProxy *m_actionProxy;
};

#endif
