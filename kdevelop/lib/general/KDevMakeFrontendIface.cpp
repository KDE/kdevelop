#include "KDevMakeFrontendIface.h"
#include "kdevmakefrontend.h"


KDevMakeFrontendIface::KDevMakeFrontendIface(KDevMakeFrontend *makeFrontend)
    : DCOPObject("KDevMakeFrontend")
{
    m_makeFrontend = makeFrontend;
}


KDevMakeFrontendIface::~KDevMakeFrontendIface()
{}


void KDevMakeFrontendIface::executeMakeCommand(const QString &command) 
{
    m_makeFrontend->executeMakeCommand(command);
}
