#include "kdevversioncontrol.h"


KDevVersionControl::KDevVersionControl(KDevApi *api, QObject *parent, const char *name)
    : KDevPart(api, parent, name)
{
}

KDevVersionControl::~KDevVersionControl()
{
}

#include "kdevversioncontrol.moc"
