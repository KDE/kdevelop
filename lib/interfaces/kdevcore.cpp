#include "KDevCoreIface.h"
#include "kdevcore.h"


KDevCore::KDevCore(QObject *parent, const char *name)
    : QObject(parent, name)
{
  new KDevCoreIface(this);
}

KDevCore::~KDevCore()
{
}

#include "kdevcore.moc"
