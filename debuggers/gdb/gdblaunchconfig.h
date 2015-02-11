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
#ifndef GDBLAUNCHCONFIG
#define GDBLAUNCHCONFIG

#include <interfaces/ilauncher.h>
#include <interfaces/launchconfigurationpage.h>
#include <outputview/outputjob.h>

class IExecutePlugin;
namespace Ui
{
class DebuggerConfigWidget;
}
namespace KDevelop
{
class OutputModel;
class ILaunchConfiguration;
class IProject;
}

namespace GDBDebugger
{
class CppDebuggerPlugin;
class DebugSession;
}

class GdbConfigPageFactory : public KDevelop::LaunchConfigurationPageFactory
{
public:
    virtual KDevelop::LaunchConfigurationPage* createWidget(QWidget* parent) override;
};

class GdbConfigPage : public KDevelop::LaunchConfigurationPage
{
Q_OBJECT
public:
    GdbConfigPage( QWidget* parent = 0 );
    virtual ~GdbConfigPage();
    virtual QIcon icon() const override;
    virtual void loadFromConfiguration(const KConfigGroup& cfg, KDevelop::IProject* = 0) override;
    virtual void saveToConfiguration(KConfigGroup, KDevelop::IProject* = 0 ) const override;
    virtual QString title() const override;
private:
    Ui::DebuggerConfigWidget* ui;
};

class GdbLauncher : public KDevelop::ILauncher
{
public:
    GdbLauncher( GDBDebugger::CppDebuggerPlugin* plugin, IExecutePlugin* execute );
    virtual QList< KDevelop::LaunchConfigurationPageFactory* > configPages() const override;
    virtual QString description() const override;
    virtual QString id() override;
    virtual QString name() const override;
    virtual KJob* start(const QString& launchMode, KDevelop::ILaunchConfiguration* cfg) override;
    virtual QStringList supportedModes() const override;
private:
    QList<KDevelop::LaunchConfigurationPageFactory*> factoryList;
    GDBDebugger::CppDebuggerPlugin* m_plugin;
    IExecutePlugin* m_execute;
};

#endif
