#ifndef _KDEVMAKEFRONTENDIFACE_H_
#define _KDEVMAKEFRONTENDIFACE_H_

#include <dcopobject.h>

class KDevMakeFrontend;

class KDevMakeFrontendIface : public DCOPObject
{
    K_DCOP
    
public:
    
    KDevMakeFrontendIface( KDevMakeFrontend *makeFrontend );
    ~KDevMakeFrontendIface();

k_dcop:
    void startMakeCommand(const QString &dir, const QString &command);

private:
    KDevMakeFrontend *m_makeFrontend;
};

#endif
