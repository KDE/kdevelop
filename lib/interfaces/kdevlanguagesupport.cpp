#include "kdevlanguagesupport.h"


KDevLanguageSupport::KDevLanguageSupport(QObject *parent, const char *name)
    : KDevPlugin(parent, name)
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
