#include "kdevlanguagesupport.h"


KDevLanguageSupport::KDevLanguageSupport(QObject *parent, const char *name)
    : KDevComponent(parent, name)
{
}

KDevLanguageSupport::~KDevLanguageSupport()
{
}

void KDevLanguageSupport::addMethodRequested(const QString &className)
{
}

void KDevLanguageSupport::addAttributeRequested(const QString &className)
{
}
