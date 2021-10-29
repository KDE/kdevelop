/*
    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "configpage.h"

#include <KConfigDialogManager>
#include <KCoreConfigSkeleton>

namespace KDevelop {

class ConfigPagePrivate
{
public:
    explicit ConfigPagePrivate(IPlugin* plugin)
        : plugin(plugin)
    {}
    QScopedPointer<KConfigDialogManager> configManager;
    KCoreConfigSkeleton* configSkeleton = nullptr;
    IPlugin* plugin;
};

ConfigPage::ConfigPage(IPlugin* plugin, KCoreConfigSkeleton* config, QWidget* parent)
    : KTextEditor::ConfigPage(parent)
    , d_ptr(new ConfigPagePrivate(plugin))
{
    setConfigSkeleton(config);
}

ConfigPage::~ConfigPage()
{
}

void ConfigPage::apply()
{
    Q_D(ConfigPage);

    // if d->configManager is null, this method must be overridden
    Q_ASSERT_X(d->configManager, metaObject()->className(),
               "Config page does not use a KConfigSkeleton, but doesn't override apply()");

    QSignalBlocker blockSigs(this); // we don't want to emit changed() while calling apply()
    d->configManager->updateSettings();
    d->configSkeleton->load();
    d->configManager->updateWidgets();
}

void ConfigPage::defaults()
{
    Q_D(ConfigPage);

    // if d->configManager is null, this method must be overridden
    Q_ASSERT_X(d->configManager, metaObject()->className(),
               "Config page does not use a KConfigSkeleton, but doesn't override defaults()");
    d->configManager->updateWidgetsDefault();
}

void ConfigPage::reset()
{
    Q_D(ConfigPage);

    // if d->configManager is null, this method must be overridden
    Q_ASSERT_X(d->configManager, metaObject()->className(),
               "Config page does not use a KConfigSkeleton, but doesn't override reset()");
    d->configManager->updateWidgets();
}

void ConfigPage::initConfigManager()
{
    Q_D(ConfigPage);

    if (d->configManager) {
        d->configManager->addWidget(this);
    }
}

KCoreConfigSkeleton* ConfigPage::configSkeleton() const
{
    Q_D(const ConfigPage);

    return d->configSkeleton;
}

void ConfigPage::setConfigSkeleton(KCoreConfigSkeleton* skel)
{
    Q_D(ConfigPage);

    if (d->configSkeleton == skel) {
        return;
    }
    d->configSkeleton = skel;
    if (!skel) {
        d->configManager.reset();
        return;
    }
    // create the config dialog manager if it didn't exist or recreate it.
    // This is needed because the used config skeleton has changed
    // and no setter for that exists in KConfigDialogManager
    d->configManager.reset(new KConfigDialogManager(this, d->configSkeleton));
    connect(d->configManager.data(), &KConfigDialogManager::widgetModified, this, &ConfigPage::changed);
    // d->configManager->addWidget(this) must be called from the config dialog,
    // since the widget tree is probably not complete when calling this function
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

IPlugin* ConfigPage::plugin() const
{
    Q_D(const ConfigPage);

    return d->plugin;
}

ConfigPage::ConfigPageType ConfigPage::configPageType() const
{
    return DefaultConfigPage;
}

} // namespace KDevelop
