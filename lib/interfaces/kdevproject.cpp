#include "kdevproject.h"


KDevProject::KDevProject(KDevApi *api, QObject *parent, const char *name)
    : KDevPart(api, parent, name)
{
}

KDevProject::~KDevProject()
{
}

#include "kdevproject.moc"
