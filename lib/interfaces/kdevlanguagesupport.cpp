#include "kdevlanguagesupport.h"


KDevLanguageSupport::KDevLanguageSupport(KDevApi *api, QObject *parent, const char *name)
    : KDevPart(api, parent, name)
{
}

KDevLanguageSupport::~KDevLanguageSupport()
{
}

KDevLanguageSupport::Features KDevLanguageSupport::features()
{
    return Features(0);
}

void KDevLanguageSupport::addClass()
{
}

void KDevLanguageSupport::addMethod(const QString &/*className*/)
{
}

void KDevLanguageSupport::addAttribute(const QString &/*className*/)
{
}

QStringList KDevLanguageSupport::fileFilters()
{
    QStringList list;
    list << "*";
    return list;
}

#include "kdevlanguagesupport.moc"
