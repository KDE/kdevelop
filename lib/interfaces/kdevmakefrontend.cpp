#include "kdevmakefrontend.h"


KDevMakeFrontend::KDevMakeFrontend(const QString& pluginName, const QString& icon, QObject *parent, const char *name)
    : KDevPlugin(pluginName, icon, parent, name ? name : "KDevMakeFrontend")
{
}

KDevMakeFrontend::~KDevMakeFrontend()
{
}

#include "kdevmakefrontend.moc"
