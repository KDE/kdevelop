/*
 * This file is part of KDevelop
 * Copyright 2014 Alex Richardson <arichardson.kde@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "configpage.h"

#include <QIcon>
#include <QVBoxLayout>

namespace KDevelop {

class ConfigPagePrivate {
public:
    ConfigPagePrivate(IPlugin* plugin, KCoreConfigSkeleton* config)
        : configSkeleton(config), plugin(plugin)
        {
        }
    QScopedPointer<KConfigDialogManager> configManager;
    KCoreConfigSkeleton* configSkeleton;
    IPlugin* plugin;
};

ConfigPage::ConfigPage(IPlugin* plugin, KCoreConfigSkeleton* config, QWidget* parent)
        : KTextEditor::ConfigPage(parent), d(new ConfigPagePrivate(plugin, config))
{
    if (d->configSkeleton) {
        d->configManager.reset(new KConfigDialogManager(parent, d->configSkeleton));
        connect(d->configManager.data(), &KConfigDialogManager::widgetModified, this, &ConfigPage::changed);
        // d->configManager->addWidget(this) must be called from the config dialog,
        // since the widget tree is not complete yet when calling this constructor
    }
}

ConfigPage::~ConfigPage()
{
}

void ConfigPage::apply()
{
    Q_ASSERT(d->configManager); // if null, this method must be overriden
    d->configManager->updateSettings();
    d->configSkeleton->load();
    d->configManager->updateWidgets();
}

void ConfigPage::defaults()
{
    Q_ASSERT(d->configManager); // if null, this method must be overriden
    d->configManager->updateWidgetsDefault();
}

void ConfigPage::reset()
{
    Q_ASSERT(d->configManager); // if null, this method must be overriden
    d->configManager->updateWidgets();
}

void ConfigPage::initConfigManager()
{
    if (d->configManager) {
        d->configManager->addWidget(this);
    }
}

KCoreConfigSkeleton* ConfigPage::configSkeleton()
{
    return d->configSkeleton;
}

void ConfigPage::setConfigSkeleton(KCoreConfigSkeleton* skel)
{
    if (d->configSkeleton == skel || !skel) {
        return;
    }
    d->configSkeleton = skel;
    // create the config dialog manager if it didn't exist or recreate it.
    // This is needed because the used config skeleton has changed
    // and no setter for that exists in KConfigDialogManager
    d->configManager.reset(new KConfigDialogManager(this, d->configSkeleton));
    connect(d->configManager.data(), &KConfigDialogManager::widgetModified, this, &ConfigPage::changed);
}


int ConfigPage::childPages() const
{
    return 0;
}

ConfigPage* ConfigPage::childPage(int number)
{
    Q_UNUSED(number)
    return nullptr;
}

IPlugin* ConfigPage::plugin()
{
    return d->plugin;
}

} // namespace KDevelop
