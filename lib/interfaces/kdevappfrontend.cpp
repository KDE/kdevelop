#include "kdevappfrontend.h"


KDevAppFrontend::KDevAppFrontend(QObject *parent, const char *name)
    : KDevPlugin(parent, name ? name : "KDevAppFrontend")
{
}

KDevAppFrontend::~KDevAppFrontend()
{
}

#include "kdevappfrontend.moc"
