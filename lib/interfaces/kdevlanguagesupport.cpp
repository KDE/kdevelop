#include "kdevlanguagesupport.h"

KDevLanguageSupport::KDevLanguageSupport( const QString& pluginName, const QString& icon, QObject *parent, const char *name)
    : KDevPlugin( pluginName, icon, parent, name ? name : "KDevLanguageSupport" )
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

QStringList KDevLanguageSupport::subclassWidget(const QString& /*formName*/)
{
    return QStringList();
}

QStringList KDevLanguageSupport::updateWidget(const QString& /*formName*/, const QString& /*fileName*/)
{
    return QStringList();
}

#include "kdevlanguagesupport.moc"
