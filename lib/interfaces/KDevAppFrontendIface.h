#ifndef _KDEVAPPFRONTENDIFACE_H_
#define _KDEVAPPFRONTENDIFACE_H_

#include <dcopobject.h>

class KDevAppFrontend;

class KDevAppFrontendIface : public DCOPObject
{
    K_DCOP
    
public:
    
    KDevAppFrontendIface( KDevAppFrontend *appFrontend );
    ~KDevAppFrontendIface();

k_dcop:
    void startAppCommand(const QString &command, bool inTerminal);

private:
    KDevAppFrontend *m_appFrontend;
};

#endif
