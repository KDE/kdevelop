#include "%{APPNAMELC}.h"

#include <QDebug>

#include <KPluginFactory>

K_PLUGIN_FACTORY_WITH_JSON(%{APPNAME}Factory, "%{APPNAMELC}.json", registerPlugin<%{APPNAME}>(); )

%{APPNAME}::%{APPNAME}(QObject *parent, const QVariantList& args)
    : KDevelop::IPlugin(QStringLiteral("%{APPNAMELC}"), parent)
{
    Q_UNUSED(args);

    qDebug() << "Hello world, my plugin is loaded!";
}

// needed for QObject class created from K_PLUGIN_FACTORY_WITH_JSON
#include "%{APPNAMELC}.moc"
