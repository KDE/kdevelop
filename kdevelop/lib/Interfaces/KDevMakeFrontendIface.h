#ifndef _KDEVMAKEFRONTENDIFACE_H_
#define _KDEVMAKEFRONTENDIFACE_H_

#include <dcopobject.h>

class KDevMakeFrontend;

class KDevMakeFrontendIface : public DCOPObject
{
    K_DCOP
    
public:
    
    KDevMakeFrontendIface( KDevMakeFrontend *editorManager );
    ~KDevMakeFrontendIface();

k_dcop:
    void executeMakeCommand(const QString &command);

private:
    KDevMakeFrontend *m_makeFrontend;
};

#endif
