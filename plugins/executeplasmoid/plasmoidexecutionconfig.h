/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMOIDEXECUTIONCONFIG_H
#define PLASMOIDEXECUTIONCONFIG_H

#include <interfaces/launchconfigurationtype.h>
#include <interfaces/launchconfigurationpage.h>
#include <interfaces/ilauncher.h>
#include <interfaces/ilaunchmode.h>

#include "ui_plasmoidexecutionconfig.h"

class ExecutePlasmoidPlugin;

class PlasmoidExecutionConfig : public KDevelop::LaunchConfigurationPage, Ui::PlasmoidExecutionPage
{
Q_OBJECT
public:
    explicit PlasmoidExecutionConfig( QWidget* parent );
    void loadFromConfiguration( const KConfigGroup& cfg, KDevelop::IProject* project = nullptr ) override;
    void saveToConfiguration( KConfigGroup cfg, KDevelop::IProject* project = nullptr ) const override;
    QString title() const override;
    QIcon icon() const override;
};

class PlasmoidLauncher : public KDevelop::ILauncher
{
public:
    explicit PlasmoidLauncher( ExecutePlasmoidPlugin* plugin );
    QList< KDevelop::LaunchConfigurationPageFactory* > configPages() const override;
    QString description() const override;
    QString id() override;
    QString name() const override;
    KJob* start(const QString& launchMode, KDevelop::ILaunchConfiguration* cfg) override;
    QStringList supportedModes() const override;
    
    static KJob* calculateDependencies(KDevelop::ILaunchConfiguration* cfg);
private:
    ExecutePlasmoidPlugin* m_plugin;
};

class PlasmoidPageFactory : public KDevelop::LaunchConfigurationPageFactory
{
public:
    PlasmoidPageFactory();
    KDevelop::LaunchConfigurationPage* createWidget(QWidget* parent) override;
};

/**
 * A specific configuration to start a launchable, this could be a native
 * compiled application, or some script file or byte-compiled file or something else
 * Provides access to the various configured information, as well as its type and a name
 */
class PlasmoidExecutionConfigType : public KDevelop::LaunchConfigurationType
{
Q_OBJECT
public:
    PlasmoidExecutionConfigType();
    ~PlasmoidExecutionConfigType() override;

    static QString typeId();
    QString id() const override { return typeId(); }
    QString name() const override;
    QList<KDevelop::LaunchConfigurationPageFactory*> configPages() const override;  
    QIcon icon() const override;
    bool canLaunch( const QUrl &file ) const override;
    bool canLaunch(KDevelop::ProjectBaseItem* item) const override;
    void configureLaunchFromItem(KConfigGroup config, KDevelop::ProjectBaseItem* item) const override;
    void configureLaunchFromCmdLineArguments(KConfigGroup config, const QStringList& args) const override;
    
    QMenu* launcherSuggestions() override;

private:
    QList<KDevelop::LaunchConfigurationPageFactory*> factoryList;

public Q_SLOTS:
    void suggestionTriggered();
};
#endif

