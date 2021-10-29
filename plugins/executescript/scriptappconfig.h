/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SCRIPTAPPCONFIGTYPE_H
#define KDEVPLATFORM_PLUGIN_SCRIPTAPPCONFIGTYPE_H

#include <interfaces/launchconfigurationtype.h>
#include <interfaces/launchconfigurationpage.h>
#include <interfaces/ilauncher.h>
#include <interfaces/ilaunchmode.h>

#include "ui_scriptappconfig.h"

class ExecuteScriptPlugin;

class ScriptAppConfigPage : public KDevelop::LaunchConfigurationPage, Ui::ScriptAppPage
{
Q_OBJECT
public:
    explicit ScriptAppConfigPage( QWidget* parent );
    void loadFromConfiguration( const KConfigGroup& cfg, KDevelop::IProject* project = nullptr ) override;
    void saveToConfiguration( KConfigGroup cfg, KDevelop::IProject* project = nullptr ) const override;
    QString title() const override;
    QIcon icon() const override;
};

class ScriptAppLauncher : public KDevelop::ILauncher
{
public:
    explicit ScriptAppLauncher( ExecuteScriptPlugin* );
    QList< KDevelop::LaunchConfigurationPageFactory* > configPages() const override;
    QString description() const override;
    QString id() override;
    QString name() const override;
    KJob* start(const QString& launchMode, KDevelop::ILaunchConfiguration* cfg) override;
    QStringList supportedModes() const override;
private:
    ExecuteScriptPlugin* m_plugin;
};

class ScriptAppPageFactory : public KDevelop::LaunchConfigurationPageFactory
{
public:
    ScriptAppPageFactory();
    KDevelop::LaunchConfigurationPage* createWidget(QWidget* parent) override;
};

/**
 * A specific configuration to start a launchable, this could be a native
 * compiled application, or some script file or byte-compiled file or something else
 * Provides access to the various configured information, as well as its type and a name
 */
class ScriptAppConfigType : public KDevelop::LaunchConfigurationType
{
    Q_OBJECT
public:
    ScriptAppConfigType();
    ~ScriptAppConfigType() override;

    static QString sharedId();

    QString id() const override;
    QString name() const override;
    QList<KDevelop::LaunchConfigurationPageFactory*> configPages() const override;
    QIcon icon() const override;
    bool canLaunch( const QUrl& file ) const override;
    bool canLaunch(KDevelop::ProjectBaseItem* item) const override;
    void configureLaunchFromItem(KConfigGroup config, KDevelop::ProjectBaseItem* item) const override;
    void configureLaunchFromCmdLineArguments(KConfigGroup config, const QStringList& args) const override;

private:
    QList<KDevelop::LaunchConfigurationPageFactory*> factoryList;
};
#endif

