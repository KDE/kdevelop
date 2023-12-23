/*
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "pluginpreferences.h"

#include <QVBoxLayout>

#include <KPluginWidget>
#include <KConfigGroup>

#include <interfaces/isession.h>

#include "../core.h"
#include "../plugincontroller.h"
#include "debug.h"


namespace KDevelop
{

PluginPreferences::PluginPreferences(QWidget* parent)
    : ConfigPage(nullptr, nullptr, parent)
{
    auto* lay = new QVBoxLayout(this );
    lay->setContentsMargins(0, 0, 0, 0);
    selector = new KPluginWidget( this );
    KConfigGroup cfgGroup(Core::self()->activeSession()->config(), QStringLiteral("Plugins"));
    selector->setConfig(cfgGroup);
    lay->addWidget( selector );
    QMap<QString, QVector<KPluginMetaData>> plugins;
    const QMap<QString, QString> categories = {
        { QStringLiteral("Core"),               i18nc("@title:group", "Core") },
        { QStringLiteral("Project Management"), i18nc("@title:group", "Project Management") },
        { QStringLiteral("Version Control"),    i18nc("@title:group", "Version Control") },
        { QStringLiteral("Utilities"),          i18nc("@title:group", "Utilities") },
        { QStringLiteral("Documentation"),      i18nc("@title:group", "Documentation") },
        { QStringLiteral("Language Support"),   i18nc("@title:group", "Language Support") },
        { QStringLiteral("Debugging"),          i18nc("@title:group", "Debugging") },
        { QStringLiteral("Testing"),            i18nc("@title:group", "Testing") },
        { QStringLiteral("Analyzers"),          i18nc("@title:group", "Analyzers") },
        { QStringLiteral("Runtimes"),           i18nc("@title:group", "Runtimes") },
        { QStringLiteral("Other"),              i18nc("@title:group", "Other") }
    };
    const auto pluginInfos = Core::self()->pluginControllerInternal()->allPluginInfos();
    for (const KPluginMetaData& info : pluginInfos) {
        const QString loadMode = info.value(QStringLiteral("X-KDevelop-LoadMode"));
        if( loadMode.isEmpty() || loadMode == QLatin1String("UserSelectable") )
        {
            QString category = info.category();
            if (!categories.contains(category)) {
                if (!category.isEmpty()) {
                    qCWarning(SHELL) << "unknown category for plugin" << info.name() << ":" << info.category();
                }
                category = QStringLiteral("Other");
            }
            plugins[category] << info;
        } else
            qCDebug(SHELL) << "skipping..." << info.pluginId() << info.value(QStringLiteral("X-KDevelop-Category")) << loadMode;
    }

    for (auto it = plugins.constBegin(), end = plugins.constEnd(); it != end; ++it) {
        selector->addPlugins(it.value(), categories.value(it.key()));
    }
    connect(selector, &KPluginWidget::changed, this, &PluginPreferences::changed);
}

void PluginPreferences::defaults()
{
    Core::self()->pluginControllerInternal()->resetToDefaults();
    selector->load();
}

void PluginPreferences::apply()
{
    selector->save();
    qCDebug(SHELL) << "Plugins before apply: " << Core::self()->pluginControllerInternal()->allPluginNames();
    Core::self()->pluginControllerInternal()->updateLoadedPlugins();
    qCDebug(SHELL) << "Plugins after apply: " << Core::self()->pluginControllerInternal()->allPluginNames();
    selector->load(); // Some plugins may have failed to load, they must be unchecked.
}

void PluginPreferences::reset()
{
    selector->load();
}

}

#include "moc_pluginpreferences.cpp"
