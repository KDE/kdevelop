#include "kdevdifffrontend.h"


KDevDiffFrontend::KDevDiffFrontend(const QString& pluginName, const QString& icon, QObject *parent, const char *name)
    : KDevPlugin(pluginName, icon, parent, name ? name : "KDevDiffFrontend")
{
}

KDevDiffFrontend::~KDevDiffFrontend()
{
}

#include "kdevdifffrontend.moc"
