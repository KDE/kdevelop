#ifndef _KDEVELOPIFACE_H_
#define _KDEVELOPIFACE_H_

#include <dcopobject.h>
#include <dcopref.h>

class KDevelopCore;
class KDCOPActionProxy;
class Project;


class KDevelopIface : public DCOPObject
{
    K_DCOP
    
public:
    KDevelopIface( KDevelopCore *core );
    ~KDevelopIface();

k_dcop:
    bool openProjectSpace(const QString &fileName);
    void closeProjectSpace();

    /** They have changed the subproject they are using, we should notify
        all the components, to give them a chance to react. */
    void changeProjectSpace ();

    QCStringList actions();
    DCOPRef action(const QCString &name);

private:
    KDevelopCore *m_core;
    KDCOPActionProxy *m_actionProxy;
};

#endif
