#include "kdevappfrontend.h"


KDevAppFrontend::KDevAppFrontend(const QString& pluginName, const QString& icon, QObject *parent, const char *name)
    : KDevPlugin(pluginName, icon, parent, name ? name : "KDevAppFrontend")
{
}

KDevAppFrontend::~KDevAppFrontend()
{
}

#include "kdevappfrontend.moc"
