#include "kdevlanguagesupport.h"


KDevLanguageSupport::KDevLanguageSupport(QObject *parent, const char *name)
    : KDevPlugin(parent, name ? name : "KDevLanguageSupport")
{
}

KDevLanguageSupport::~KDevLanguageSupport()
{
}

KDevLanguageSupport::Features KDevLanguageSupport::features()
{
    return Features(0);
}

QStringList KDevLanguageSupport::fileFilters()
{
    QStringList list;
    list << "*";
    return list;
}

QString KDevLanguageSupport::formatClassName(const QString &name)
{
    return name;
}

QString KDevLanguageSupport::unformatClassName(const QString &name)
{
    return name;
}

void KDevLanguageSupport::addClass()
{
}

void KDevLanguageSupport::addMethod(const QString &/*className*/)
{
}

void KDevLanguageSupport::implementVirtualMethods(const QString& /*className*/ )
{
}

void KDevLanguageSupport::addAttribute(const QString &/*className*/)
{
}

QStringList KDevLanguageSupport::subclassWidget(QString formName)
{
    return QStringList();
}

QStringList KDevLanguageSupport::updateWidget(QString formName, QString fileName)
{
    return QStringList();
}

#include "kdevlanguagesupport.moc"
