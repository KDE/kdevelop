#include "kdevmakefrontend.h"


KDevMakeFrontend::KDevMakeFrontend(QObject *parent, const char *name)
    : KDevPlugin(parent, name ? name : "KDevMakeFrontend")
{
}

KDevMakeFrontend::~KDevMakeFrontend()
{
}

#include "kdevmakefrontend.moc"
