/*
 * Common code for MI debugger support
 *
 * Copyright 1999-2001 John Birch <jbb@kdevelop.org>
 * Copyright 2001 by Bernd Gehrmann <bernd@kdevelop.org>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2016  Aetf <aetf@unlimitedcodeworks.xyz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MIDEBUGGERPLUGIN_H
#define MIDEBUGGERPLUGIN_H

#include <interfaces/iplugin.h>
#include <interfaces/istatus.h>
#include <interfaces/iuicontroller.h>
#include <sublime/view.h>

#include <QHash>

class QDBusInterface;
class QSignalMapper;
class QUrl;

namespace KDevelop {
class Context;
}

namespace KDevMI {
class MIDebugSession;
class MIDebuggerPlugin : public KDevelop::IPlugin, public KDevelop::IStatus
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IStatus)

public:
    MIDebuggerPlugin(const QString& componentName, QObject *parent);
    ~MIDebuggerPlugin() override;

    void unload() override;
    KDevelop::ContextMenuExtension contextMenuExtension( KDevelop::Context* ) override;

    virtual MIDebugSession *createSession() const = 0;

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
    void stopDebugger();
    void attachTo(int pid);
    void coreFile(const QString& core);
    void runUntil(const QUrl &url, int line);
    void jumpTo(const QUrl &url, int line);
    void addWatchVariable(const QString& var);
    void evaluateExpression(const QString& expr);
    void raiseDebuggerConsoleViews();

protected Q_SLOTS:

    void slotDebugExternalProcess(QObject* interface);
    void slotExamineCore();
    #ifdef KDEV_ENABLE_DBG_ATTACH_DIALOG
    void slotAttachProcess();
    #endif

    void slotDBusServiceRegistered(const QString& service);
    void slotDBusServiceUnregistered(const QString& service);
    void slotCloseDrKonqi();

protected:
    void setupToolviews();
    void setupActions();
    void setupDBus();

    void attachProcess(int pid);
    void showStatusMessage(const QString& msg, int timeout);

private:
    QHash<QString, QDBusInterface*> m_drkonqis;
    QSignalMapper* m_drkonqiMap;
    QString m_drkonqi;
};

template<class T, class Plugin = MIDebuggerPlugin>
class DebuggerToolFactory : public KDevelop::IToolViewFactory
{
public:
    DebuggerToolFactory(Plugin * plugin, const QString &id, Qt::DockWidgetArea defaultArea)
    : m_plugin(plugin), m_id(id), m_defaultArea(defaultArea)
    {}

    QWidget* create(QWidget *parent = 0) override
    {
        return new T(m_plugin, parent);
    }

    QString id() const override
    {
        return m_id;
    }

    Qt::DockWidgetArea defaultPosition() override
    {
        return m_defaultArea;
    }

    void viewCreated(Sublime::View* view) override
    {
        if (view->widget()->metaObject()->indexOfSignal(QMetaObject::normalizedSignature("requestRaise()")) != -1)
            QObject::connect(view->widget(), SIGNAL(requestRaise()), view, SLOT(requestRaise()));
    }

    /* At present, some debugger widgets (e.g. breakpoint) contain actions so that shortcuts
        work, but they don't need any toolbar.  So, suppress toolbar action.  */
    QList<QAction*> toolBarActions(QWidget* viewWidget) const override
    {
        Q_UNUSED(viewWidget);
        return QList<QAction*>();
    }

private:
    Plugin * m_plugin;
    QString m_id;
    Qt::DockWidgetArea m_defaultArea;
};

} // end of namespace KDevMI

#endif // MIDEBUGGERPLUGIN_H
