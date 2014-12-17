/*
 * GDB Debugger Support
 *
 * Copyright 1999-2001 John Birch <jbb@kdevelop.org>
 * Copyright 2001 by Bernd Gehrmann <bernd@kdevelop.org>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
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

#ifndef _DEBUGGERPART_H_
#define _DEBUGGERPART_H_

#include <QByteArray>
#include <QLabel>
#include <QtCore/QVariant>

#include <KConfigGroup>
#include <KTextEditor/Cursor>
#include <QUrl>

#include <interfaces/iplugin.h>
#include <interfaces/istatus.h>

class QLabel;
class QMenu;
class QDBusInterface;
class QSignalMapper;
class ProcessWidget;

class KToolBar;
class QAction;

namespace KDevelop {
class Context;
class ProcessLineMaker;
}

namespace GDBDebugger
{
class DisassembleWidget;
class Breakpoint;
class GDBOutputWidget;
class MemoryViewerWidget;
class DebugSession;
template<typename T> class DebuggerToolFactory;

class CppDebuggerPlugin : public KDevelop::IPlugin, public KDevelop::IStatus
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IStatus)

public:
    friend class DebugSession;

    CppDebuggerPlugin( QObject *parent, const QVariantList & = QVariantList() );
    ~CppDebuggerPlugin();

    virtual void unload();
    
    virtual KDevelop::ContextMenuExtension contextMenuExtension( KDevelop::Context* );
    
    DebugSession *createSession();

public:
    //BEGIN IStatus
    virtual QString statusName() const;

Q_SIGNALS:
    void clearMessage(KDevelop::IStatus*);
    void showMessage(KDevelop::IStatus*, const QString & message, int timeout = 0);
    void hideProgress(KDevelop::IStatus*);
    void showProgress(KDevelop::IStatus*, int minimum, int maximum, int value);
    void showErrorMessage(const QString&, int);
    //END IStatus

    void addWatchVariable(const QString& variable);
    void evaluateExpression(const QString& variable);

    void raiseGdbConsoleViews();

    void reset();

private Q_SLOTS:
    void setupDBus();
    void slotDebugExternalProcess(QObject* interface);
    void contextEvaluate();
    void contextWatch();

    void slotExamineCore();
    #ifdef KDEV_ENABLE_GDB_ATTACH_DIALOG
    void slotAttachProcess();
    #endif

    void slotDBusServiceRegistered(const QString& service);
    void slotDBusServiceUnregistered(const QString& service);

    void slotCloseDrKonqi();

    void slotFinished();

    void controllerMessage(const QString&, int);

Q_SIGNALS:
    //TODO: port to launch framework
    //void startDebugger(const KDevelop::IRun & run, KJob* job);
    void stopDebugger();
    void attachTo(int pid);
    void coreFile(const QString& core);
    void runUntil(const QUrl &url, int line);
    void jumpTo(const QUrl &url, int line);

protected:
    virtual void initializeGuiState();

private:
    KConfigGroup config() const;

    void attachProcess(int pid);
    void setupActions();

    QHash<QString, QDBusInterface*> m_drkonqis;
    QSignalMapper* m_drkonqiMap;
    QString m_drkonqi;

    QString m_contextIdent;

    KConfigGroup m_config;

    DebuggerToolFactory< DisassembleWidget >* disassemblefactory;
    DebuggerToolFactory< GDBOutputWidget >* gdbfactory;
    DebuggerToolFactory< MemoryViewerWidget >* memoryviewerfactory;
};

}

#endif
