#include "kdevproject.h"


KDevProject::KDevProject( const QString& pluginName, const QString& icon, QObject *parent, const char *name)
    : KDevPlugin( pluginName, icon, parent, name)
{
}

KDevProject::~KDevProject()
{
}

#include "kdevproject.moc"
