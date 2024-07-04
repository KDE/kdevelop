#include "%{APPNAMELC}.h"

#include <debug.h>

#include <KPluginFactory>

K_PLUGIN_FACTORY_WITH_JSON(%{APPNAMEID}Factory, "%{APPNAMELC}.json", registerPlugin<%{APPNAMEID}>(); )

%{APPNAMEID}::%{APPNAMEID}(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args)
    : KDevelop::IPlugin(QStringLiteral("%{APPNAMELC}"), parent, metaData)
{
    Q_UNUSED(args);

    qCDebug(PLUGIN_%{APPNAMEUC}) << "Hello world, my plugin is loaded!";
}

// needed for QObject class created from K_PLUGIN_FACTORY_WITH_JSON
#include "%{APPNAMELC}.moc"
#include "moc_%{APPNAMELC}.cpp"
