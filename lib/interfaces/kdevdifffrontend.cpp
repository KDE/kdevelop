#include "kdevdifffrontend.h"


KDevDiffFrontend::KDevDiffFrontend(QObject *parent, const char *name)
    : KDevPlugin(parent, name ? name : "KDevDiffFrontend")
{
}

KDevDiffFrontend::~KDevDiffFrontend()
{
}

#include "kdevdifffrontend.moc"
