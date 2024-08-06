/*
    SPDX-FileCopyrightText: 2006 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GDBCONFIGPAGE_H
#define GDBCONFIGPAGE_H

#include <interfaces/ilauncher.h>
#include <interfaces/launchconfigurationpage.h>

#include <QPointer>

class IExecutePlugin;
namespace Ui
{
class GdbConfigPage;
}
namespace KDevelop
{
class ILaunchConfiguration;
class IProject;
}

namespace KDevMI
{
namespace GDB
{
class CppDebuggerPlugin;
}
}

class GdbConfigPageFactory : public KDevelop::LaunchConfigurationPageFactory
{
public:
    KDevelop::LaunchConfigurationPage* createWidget(QWidget* parent) override;
};

class GdbConfigPage : public KDevelop::LaunchConfigurationPage
{
Q_OBJECT
public:
    explicit GdbConfigPage( QWidget* parent = nullptr );
    ~GdbConfigPage() override;
    QIcon icon() const override;
    void loadFromConfiguration(const KConfigGroup& cfg, KDevelop::IProject* = nullptr) override;
    void saveToConfiguration(KConfigGroup, KDevelop::IProject* = nullptr ) const override;
    QString title() const override;
private:
    Ui::GdbConfigPage* ui;
};

class GdbLauncher : public KDevelop::ILauncher
{
public:
    GdbLauncher( KDevMI::GDB::CppDebuggerPlugin* plugin, IExecutePlugin* execute );
    ~GdbLauncher() override;
    QList< KDevelop::LaunchConfigurationPageFactory* > configPages() const override;
    QString description() const override;
    QString id() override;
    QString name() const override;
    KJob* start(const QString& launchMode, KDevelop::ILaunchConfiguration* cfg) override;
    QStringList supportedModes() const override;
private:
    QList<KDevelop::LaunchConfigurationPageFactory*> factoryList;
    QPointer<KDevMI::GDB::CppDebuggerPlugin> m_plugin;
    IExecutePlugin* m_execute;
};

#endif
