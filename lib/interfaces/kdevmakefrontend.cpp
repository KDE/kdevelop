#include "kdevmakefrontend.h"


KDevMakeFrontend::KDevMakeFrontend(KDevApi *api, QObject *parent, const char *name)
    : KDevPart(api, parent, name)
{
}

KDevMakeFrontend::~KDevMakeFrontend()
{
}

#include "kdevmakefrontend.moc"
