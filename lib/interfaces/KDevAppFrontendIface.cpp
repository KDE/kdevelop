#include "KDevAppFrontendIface.h"
#include "kdevappfrontend.h"


KDevAppFrontendIface::KDevAppFrontendIface(KDevAppFrontend *appFrontend)
    : DCOPObject("KDevAppFrontend")
{
    m_appFrontend = appFrontend;
}


KDevAppFrontendIface::~KDevAppFrontendIface()
{}


void KDevAppFrontendIface::startAppCommand(const QString &command) 
{
    m_appFrontend->startAppCommand(command);
}
