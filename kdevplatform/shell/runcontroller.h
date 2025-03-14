/*
    SPDX-FileCopyrightText: 2007-2008 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_RUNCONTROLLER_H
#define KDEVPLATFORM_RUNCONTROLLER_H

#include <QItemDelegate>

#include <KColorScheme>
#include <KStatefulBrush>

#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/iruncontroller.h>

#include "shellexport.h"

class QStyleOptionViewItem;
class QModelIndex;

namespace KDevelop
{
class Context;
class ContextMenuExtension;
class IPlugin;
class IProject;
class LaunchConfiguration;
class LaunchConfigurationType;
class RunControllerPrivate;

class KDEVPLATFORMSHELL_EXPORT RunController : public IRunController
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kdevelop.RunController")

public:
    explicit RunController(QObject *parent);
    ~RunController() override;

    void registerJob(KJob *job) override;
    void unregisterJob(KJob *job) override;
    QList<KJob*> currentJobs() const override;

    KJob* execute(const QString& launchMode, ILaunchConfiguration* launch) override;
    QList<ILaunchMode*> launchModes() const override;

    /**
     * @copydoc IRunController::addLaunchMode
     */
    void addLaunchMode( ILaunchMode* mode ) override;

    /**
     * @copydoc IRunController::removeLaunchMode
     */
    void removeLaunchMode( ILaunchMode* mode ) override;

    /**
     * @copydoc IRunController::launchModeForId()
     */
    KDevelop::ILaunchMode* launchModeForId(const QString& id) const override;

    void initialize();
    void cleanup();

    QItemDelegate* delegate() const;

    void addLaunchConfiguration( LaunchConfiguration* l );
    void removeLaunchConfiguration( LaunchConfiguration* l );

    QList<LaunchConfiguration*> launchConfigurationsInternal() const;
    QList<ILaunchConfiguration*> launchConfigurations() const override;
    /**
     * @copydoc IRunController::launchConfigurationTypes()
     */
    QList<LaunchConfigurationType*> launchConfigurationTypes() const override;

    /**
     * @copydoc IRunController::addConfigurationType()
     */
    void addConfigurationType( LaunchConfigurationType* type ) override;

    /**
     * @copydoc IRunController::removeConfigurationType()
     */
    void removeConfigurationType( LaunchConfigurationType* type ) override;

    /**
     * Find the launch configuration type for the given @p id.
     * @returns the launch configuration type having the id, or 0 if no such type is known
     */
    LaunchConfigurationType* launchConfigurationTypeForId( const QString& ) override;

    ILaunchConfiguration* createLaunchConfiguration ( LaunchConfigurationType* type,
                                                              const QPair<QString,QString>& launcher,
                                                              IProject* project = nullptr,
                                                              const QString& name = QString() ) override;

    void setDefaultLaunch(ILaunchConfiguration* l);
    LaunchConfiguration* defaultLaunch() const;

    /**
     * @copydoc IRunController::showConfigurationDialog()
     */
    void showConfigurationDialog() const override;

    ContextMenuExtension contextMenuExtension(KDevelop::Context* ctx, QWidget* parent);

public Q_SLOTS:
    Q_SCRIPTABLE void executeDefaultLaunch(const QString& runMode) override;

    Q_SCRIPTABLE void stopAllProcesses() override;

protected Q_SLOTS:
    void finished(KJob *job) override;
    void suspended(KJob *job) override;
    void resumed(KJob *job) override;

private Q_SLOTS:
    void slotExecute();
    void slotDebug();
    void slotProfile();
    void slotProjectOpened(KDevelop::IProject* project);
    void slotProjectClosing(KDevelop::IProject* project);
    void slotKillJob();
    void launchChanged(LaunchConfiguration*);
    void jobPercentChanged();

private:
    void setupActions();
    void checkState();
    void removeLaunchConfigurationInternal( LaunchConfiguration* l );

private:
    const QScopedPointer<class RunControllerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(RunController)
};

class RunDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit RunDelegate( QObject* = nullptr );
    void paint(QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const override;
private:
    KStatefulBrush runProviderBrush;
    KStatefulBrush errorBrush;
};

}

#endif
