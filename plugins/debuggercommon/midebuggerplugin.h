/*
    SPDX-FileCopyrightText: 1999-2001 John Birch <jbb@kdevelop.org>
    SPDX-FileCopyrightText: 2001 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef MIDEBUGGERPLUGIN_H
#define MIDEBUGGERPLUGIN_H

#include <config-kdevelop.h>

#include <interfaces/iplugin.h>
#include <interfaces/istatus.h>

#include <QHash>

#include <memory>

class QDBusServiceWatcher;

namespace KDevelop {
class IDebugSession;
}

namespace KDevMI {
class IToolViewFactoryHolder;
class MIAttachProcessJob;
class MIDebugSession;
class DBusProxy;

class MIDebuggerPlugin : public KDevelop::IPlugin, public KDevelop::IStatus
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IStatus)

public:
    MIDebuggerPlugin(const QString& componentName, const QString& displayName, QObject* parent,
                     const KPluginMetaData& metaData);
    ~MIDebuggerPlugin() override;

    void unload() override;

    MIDebugSession* createSession();

//BEGIN IStatus
public:
    QString statusName() const override;

Q_SIGNALS:
    void clearMessage(KDevelop::IStatus*) override;
    void showMessage(KDevelop::IStatus*, const QString & message, int timeout = 0) override;
    void hideProgress(KDevelop::IStatus*) override;
    void showProgress(KDevelop::IStatus*, int minimum, int maximum, int value) override;
    void showErrorMessage(const QString&, int) override;
//END IStatus

Q_SIGNALS:
    void reset();
    void raiseDebuggerConsoleViews();

protected Q_SLOTS:

    void slotDebugExternalProcess(DBusProxy* proxy);
    void slotExamineCore();

#if HAVE_KSYSGUARD
    void slotAttachProcess();
#endif

protected:
    using ToolViewFactoryHolderPtr = std::unique_ptr<IToolViewFactoryHolder>;

    void setupActions();
    void setupDBus();

    MIAttachProcessJob* attachProcess(int pid);
    void showStatusMessage(const QString& msg, int timeout);

private:
    [[nodiscard]] virtual MIDebugSession* createSessionObject() = 0;
    [[nodiscard]] virtual ToolViewFactoryHolderPtr createToolViewFactoryHolder() = 0;

    [[nodiscard]] ToolViewFactoryHolderPtr reuseOrCreateToolViewFactoryHolder(KDevelop::IDebugSession* previousSession);

    QHash<QString, DBusProxy*> m_drkonqis;
    const QString m_displayName;
    QDBusServiceWatcher* m_watcher = nullptr;
};

} // end of namespace KDevMI

#endif // MIDEBUGGERPLUGIN_H
