/*  This file is part of KDevelop
    Copyright 2009 Andreas Pakulat <apaku@gmx.de>
    Copyright 2009 Niko Sams <niko.sams@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef SCRIPTAPPCONFIGTYPE_H
#define SCRIPTAPPCONFIGTYPE_H

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
    PlasmoidExecutionConfig( QWidget* parent );
    void loadFromConfiguration( const KConfigGroup& cfg, KDevelop::IProject* project = 0 );
    void saveToConfiguration( KConfigGroup cfg, KDevelop::IProject* project = 0 ) const;
    QString title() const;
    KIcon icon() const;

private slots:
    void addDep();
    void removeDep();
    void moveDependencyDown();
    void moveDependencyUp();
    void checkActions(const QItemSelection & selected, const QItemSelection& deselected);
    void depEdited(const QString& newtext);
};

class PlasmoidLauncher : public KDevelop::ILauncher
{
public:
    PlasmoidLauncher( ExecutePlasmoidPlugin* plugin );
    virtual QList< KDevelop::LaunchConfigurationPageFactory* > configPages() const;
    virtual QString description() const;
    virtual QString id();
    virtual QString name() const;
    virtual KJob* start(const QString& launchMode, KDevelop::ILaunchConfiguration* cfg);
    virtual KJob* dependencies(KDevelop::ILaunchConfiguration* cfg);
    virtual QStringList supportedModes() const;
    
    static KJob* calculateDependencies(KDevelop::ILaunchConfiguration* cfg);
private:
    ExecutePlasmoidPlugin* m_plugin;
};

class PlasmoidPageFactory : public KDevelop::LaunchConfigurationPageFactory
{
public:
    PlasmoidPageFactory();
    virtual KDevelop::LaunchConfigurationPage* createWidget(QWidget* parent);
};

/**
 * A specific configuration to start a launchable, this could be a native
 * compiled application, or some script file or byte-compiled file or something else
 * Provides access to the various configured informations, as well as its type and a name
 */
class PlasmoidExecutionConfigType : public KDevelop::LaunchConfigurationType
{
Q_OBJECT
public:
    PlasmoidExecutionConfigType();
    virtual ~PlasmoidExecutionConfigType();

    static QString typeId();
    QString id() const { return typeId(); }
    QString name() const;
    QList<KDevelop::LaunchConfigurationPageFactory*> configPages() const;  
    KIcon icon() const;
    virtual bool canLaunch( const KUrl& file ) const;
    virtual bool canLaunch(KDevelop::ProjectBaseItem* item) const;
    virtual void configureLaunchFromItem(KConfigGroup config, KDevelop::ProjectBaseItem* item) const;
    virtual void configureLaunchFromCmdLineArguments(KConfigGroup config, const QStringList& args) const;
    
    virtual QMenu* launcherSuggestions();

private:
    QList<KDevelop::LaunchConfigurationPageFactory*> factoryList;

public slots:
    void suggestionTriggered();
};
#endif

