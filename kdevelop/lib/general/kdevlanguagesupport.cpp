#include "kdevlanguagesupport.h"


KDevLanguageSupport::KDevLanguageSupport(QObject *parent, const char *name)
    : KDevComponent(parent, name)
{
}

KDevLanguageSupport::~KDevLanguageSupport()
{
}

bool KDevLanguageSupport::hasFeature(Features /*feature*/)
{
    return false;
}

void KDevLanguageSupport::newClassRequested()
{
}

void KDevLanguageSupport::addMethodRequested(const QString &/*className*/)
{
}

void KDevLanguageSupport::addAttributeRequested(const QString &/*className*/)
{
}

QStringList KDevLanguageSupport::fileFilters(){
  QStringList list;
  list << "*";
  return list;
}

#include "kdevlanguagesupport.moc"
