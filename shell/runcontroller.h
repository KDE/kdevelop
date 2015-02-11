/* This file is part of KDevelop
Copyright 2007-2008 Hamish Rodda <rodda@kde.org>

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

#ifndef KDEVPLATFORM_RUNCONTROLLER_H
#define KDEVPLATFORM_RUNCONTROLLER_H

#include <QtCore/QPointer>
#include <QItemDelegate>

#include <kcolorscheme.h>

#include <interfaces/iruncontroller.h>

#include <interfaces/ilaunchconfiguration.h>

#include "shellexport.h"

class QStyleOptionViewItem;
class QPainter;
class QModelIndex;
class KStatefulBrush;

namespace KDevelop
{
class Context;
class ContextMenuExtension;
class IPlugin;
class IProject;
class LaunchConfiguration;
class LaunchConfigurationType;

class KDEVPLATFORMSHELL_EXPORT RunController : public IRunController
{
    Q_OBJECT

public:
    RunController(QObject *parent);
    ~RunController();
    
    static QString LaunchConfigurationsGroup;
    static QString LaunchConfigurationsListEntry;

    virtual void registerJob(KJob *job) override;
    virtual void unregisterJob(KJob *job) override;
    virtual QList<KJob*> currentJobs() const override;

    KJob* execute(const QString& launchMode, ILaunchConfiguration* launch) override;
    LaunchConfiguration* defaultLaunch() const;
    QList<ILaunchMode*> launchModes() const override;
    
    /**
     * @copydoc IRunController::addLaunchMode
     */
    virtual void addLaunchMode( ILaunchMode* mode ) override;
    
    /**
     * @copydoc IRunController::removeLaunchMode
     */
    virtual void removeLaunchMode( ILaunchMode* mode ) override;

    /**
     * @copydoc IRunController::launchModeForId()
     */
    virtual KDevelop::ILaunchMode* launchModeForId(const QString& id) const override;

    void initialize();
    void cleanup();

    QItemDelegate* delegate() const;
    
    void addLaunchConfiguration( LaunchConfiguration* l );
    void removeLaunchConfiguration( LaunchConfiguration* l );
    
    QList<LaunchConfiguration*> launchConfigurationsInternal() const;
    virtual QList<ILaunchConfiguration*> launchConfigurations() const override;
    /**
     * @copydoc IRunController::launchConfigurationTypes()
     */
    virtual QList<LaunchConfigurationType*> launchConfigurationTypes() const override;

    /**
     * @copydoc IRunController::addConfigurationType()
     */
    virtual void addConfigurationType( LaunchConfigurationType* type ) override;

    /**
     * @copydoc IRunController::removeConfigurationType()
     */
    virtual void removeConfigurationType( LaunchConfigurationType* type ) override;

    /**
     * Find the launch configuration type for the given @p id.
     * @returns the launch configuration type having the id, or 0 if no such type is known
     */
    LaunchConfigurationType* launchConfigurationTypeForId( const QString& ) override;

    virtual ILaunchConfiguration* createLaunchConfiguration ( LaunchConfigurationType* type,
                                                              const QPair<QString,QString>& launcher, 
                                                              IProject* project = 0,
                                                              const QString& name = QString() ) override;
    
    virtual void executeDefaultLaunch(const QString& runMode) override;

    void setDefaultLaunch(ILaunchConfiguration* l);
    
    ContextMenuExtension contextMenuExtension( KDevelop::Context* ctx );

public Q_SLOTS:
    virtual void stopAllProcesses() override;

protected Q_SLOTS:
    virtual void finished(KJob *job) override;
    virtual void suspended(KJob *job) override;
    virtual void resumed(KJob *job) override;

private Q_SLOTS:
    void slotRefreshProject(KDevelop::IProject* project);
    void slotExecute();
    void slotDebug();
    void slotProfile();
    void slotProjectOpened(KDevelop::IProject* project);
    void slotProjectClosing(KDevelop::IProject* project);
    void slotKillJob();
    void launchChanged(LaunchConfiguration*);
    void jobDestroyed(QObject* job);

private:
    void setupActions();
    void checkState();

    Q_PRIVATE_SLOT(d, void configureLaunches())
    Q_PRIVATE_SLOT(d, void launchAs(int))

    class RunControllerPrivate;
    RunControllerPrivate* const d;
};

class RunDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    RunDelegate( QObject* = 0 );
    void paint(QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const override;
private:
    KStatefulBrush runProviderBrush;
    KStatefulBrush errorBrush;
};

}

#endif
