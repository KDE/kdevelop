#include "kdevproject.h"


KDevProject::KDevProject(QObject *parent, const char *name)
    : KDevPlugin(parent, name)
{
}

KDevProject::~KDevProject()
{
}

#include "kdevproject.moc"
