#include "kdevappfrontend.h"


KDevAppFrontend::KDevAppFrontend(KDevApi *api, QObject *parent, const char *name)
    : KDevPart(api, parent, name)
{
}

KDevAppFrontend::~KDevAppFrontend()
{
}

#include "kdevappfrontend.moc"
