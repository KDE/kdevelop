/*
* GDB Debugger Support
*
* Copyright 2006 Vladimir Prus <ghost@cs.msu.su>
* Copyright 2007 Hamish Rodda <rodda@kde.org>
* Copyright 2009 Andreas Pakulat <apaku@gmx.de>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
