#include "KDevAppFrontendIface.h"
#include "kdevappfrontend.h"


KDevAppFrontendIface::KDevAppFrontendIface(KDevAppFrontend *appFrontend)
    : DCOPObject("KDevAppFrontend")
{
    m_appFrontend = appFrontend;
}


KDevAppFrontendIface::~KDevAppFrontendIface()
{}


void KDevAppFrontendIface::startAppCommand(const QString &directory, const QString &command, bool inTerminal)
{
    m_appFrontend->startAppCommand(directory, command, inTerminal);
}
