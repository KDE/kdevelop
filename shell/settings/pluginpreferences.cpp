/* KDevelop Project Settings
 *
 * Copyright 2008 Andreas Pakulat <apaku@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "pluginpreferences.h"

#include <QDebug>
#include <QVBoxLayout>

#include <KPluginSelector>
#include <KPluginInfo>

#include <interfaces/isession.h>

#include "../core.h"
#include "../plugincontroller.h"
#include "../debug.h"


namespace KDevelop
{

PluginPreferences::PluginPreferences(QWidget* parent)
    : ConfigPage(nullptr, nullptr, parent)
{
    QVBoxLayout* lay = new QVBoxLayout(this );
    lay->setMargin(0);
    selector = new KPluginSelector( this );
    lay->addWidget( selector );
    QMap<QString, QList<KPluginInfo>> plugins;
    const QMap<QString, QString> categories = {
        { "Core", i18nc("@title:group", "Core") },
        { "Project Management", i18nc("@title:group", "Project Management") },
        { "Version Control", i18nc("@title:group", "Version Control") },
        { "Utilities", i18nc("@title:group", "Utilities") },
        { "Documentation", i18nc("@title:group", "Documentation") },
        { "Language Support", i18nc("@title:group", "Language Support") },
        { "Debugging", i18nc("@title:group", "Debugging") },
        { "Testing", i18nc("@title:group", "Testing") },
        { "Analyzers", i18nc("@title:group", "Analyzers") },
        { "Other", i18nc("@title:group", "Other") }
    };
    foreach (const KPluginMetaData& info, Core::self()->pluginControllerInternal()->allPluginInfos()) {
        const QString loadMode = info.value(QStringLiteral("X-KDevelop-LoadMode"));
        if( loadMode.isEmpty() || loadMode == QLatin1String("UserSelectable") )
        {
            QString category = info.category();
            if (!categories.contains(category)) {
                if (!category.isEmpty()) {
                    qWarning() << "unknown category for plugin" << info.name() << ":" << info.category();
                }
                category = QStringLiteral("Other");
            }
            KPluginInfo kpi(info);
            kpi.setPluginEnabled(Core::self()->pluginControllerInternal()->isEnabled(info));
            plugins[category] << kpi;
        } else
            qDebug() << "skipping..." << info.pluginId() << info.value(QStringLiteral("X-KDevelop-Category")) << loadMode;
    }

    for (auto it = plugins.constBegin(), end = plugins.constEnd(); it != end; ++it) {
        selector->addPlugins(it.value(), KPluginSelector::ReadConfigFile,
                              categories.value(it.key()), it.key(),
                              Core::self()->activeSession()->config() );
    }
    connect(selector, &KPluginSelector::changed, this, &PluginPreferences::changed);
    selector->load();
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


