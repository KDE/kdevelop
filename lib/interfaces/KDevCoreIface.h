#ifndef _KDEVCOREIFACE_H_
#define _KDEVCOREIFACE_H_

#include <dcopobject.h>
#include <dcopref.h>

class KDevCore;


class KDevCoreIface : public QObject, public DCOPObject
{
    Q_OBJECT
    K_DCOP
    
public:
    
    KDevCoreIface( KDevCore *core );
    ~KDevCoreIface();

private slots:
    void forwardProjectOpened();
    void forwardProjectClosed();
    
private:
    KDevCore *m_core;
};

#endif
