#include "kdevversioncontrol.h"


KDevVersionControl::KDevVersionControl(const QString& pluginName, const QString& icon, QObject *parent, const char *name)
    : KDevPlugin(pluginName, icon, parent, name)
{
}

KDevVersionControl::~KDevVersionControl()
{
}

#include "kdevversioncontrol.moc"
