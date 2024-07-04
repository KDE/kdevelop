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
#include <interfaces/iuicontroller.h>
#include <sublime/view.h>

#include <QHash>

class QDBusServiceWatcher;
class QUrl;

namespace KDevelop {
class Context;
}

namespace KDevMI {
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
    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent) override;

    virtual MIDebugSession *createSession() = 0;

    virtual void setupToolViews() = 0;
    /**
     * The implementation should be sure it's safe to call
     * even when tool views are already unloaded.
     */
    virtual void unloadToolViews() = 0;

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
    void addWatchVariable(const QString& var);
    void evaluateExpression(const QString& expr);
    void raiseDebuggerConsoleViews();

protected Q_SLOTS:

    void slotDebugExternalProcess(DBusProxy* proxy);
    void slotExamineCore();

#if HAVE_KSYSGUARD
    void slotAttachProcess();
#endif

protected:
    void setupActions();
    void setupDBus();

    MIAttachProcessJob* attachProcess(int pid);
    void showStatusMessage(const QString& msg, int timeout);

private:
    QHash<QString, DBusProxy*> m_drkonqis;
    const QString m_displayName;
    QDBusServiceWatcher* m_watcher = nullptr;
};

template<class T, class Plugin = MIDebuggerPlugin>
class DebuggerToolFactory : public KDevelop::IToolViewFactory
{
public:
    DebuggerToolFactory(Plugin * plugin, const QString &id, Qt::DockWidgetArea defaultArea)
    : m_plugin(plugin), m_id(id), m_defaultArea(defaultArea)
    {}

    QWidget* create(QWidget *parent = nullptr) override
    {
        return new T(m_plugin, parent);
    }

    QString id() const override
    {
        return m_id;
    }

    Qt::DockWidgetArea defaultPosition() const override
    {
        return m_defaultArea;
    }

    void viewCreated(Sublime::View* view) override
    {
        if (view->widget()->metaObject()->indexOfSignal(QMetaObject::normalizedSignature("requestRaise()").constData()) != -1)
            QObject::connect(view->widget(), SIGNAL(requestRaise()), view, SLOT(requestRaise()));
    }

private:
    Plugin * m_plugin;
    QString m_id;
    Qt::DockWidgetArea m_defaultArea;
};

} // end of namespace KDevMI

#endif // MIDEBUGGERPLUGIN_H
