#include "KDevMakeFrontendIface.h"
#include "kdevmakefrontend.h"


KDevMakeFrontendIface::KDevMakeFrontendIface(KDevMakeFrontend *makeFrontend)
    : DCOPObject("KDevMakeFrontend")
{
    m_makeFrontend = makeFrontend;
}


KDevMakeFrontendIface::~KDevMakeFrontendIface()
{}


void KDevMakeFrontendIface::startMakeCommand(const QString &dir, const QString &command) 
{
    m_makeFrontend->startMakeCommand(dir, command);
}
