/*
    SPDX-FileCopyrightText: 1999 John Birch <jbb@kdevelop.org >
    SPDX-FileCopyrightText: 2007 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "gdb.h"

#include "dbgglobal.h"
#include "debuglog.h"

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <sublime/message.h>

#include <KConfigGroup>
#include <kcoreaddons_version.h>
#include <KLocalizedString>
#include <KShell>

#include <QApplication>
#include <QFileInfo>
#include <QUrl>

using namespace KDevMI::GDB;
using namespace KDevMI::MI;

GdbDebugger::GdbDebugger(QObject* parent)
    : MIDebugger(parent)
{
}

GdbDebugger::~GdbDebugger()
{
}

bool GdbDebugger::start(KConfigGroup& config, const QStringList& extraArguments)
{
    // FIXME: verify that default value leads to something sensible
    QUrl gdbUrl = config.readEntry(Config::GdbPathEntry, QUrl());
    if (gdbUrl.isEmpty()) {
        m_debuggerExecutable = QStringLiteral("gdb");
    } else {
        // FIXME: verify its' a local path.
        m_debuggerExecutable = gdbUrl.url(QUrl::PreferLocalFile | QUrl::StripTrailingSlash);
    }

    QStringList arguments = extraArguments;
    arguments << QStringLiteral("--interpreter=mi2") << QStringLiteral("-quiet");

    QString fullCommand;

    QUrl shell = config.readEntry(Config::DebuggerShellEntry, QUrl());
    if(!shell.isEmpty()) {
        qCDebug(DEBUGGERGDB) << "have shell" << shell;
        QString shell_without_args = shell.toLocalFile().split(QLatin1Char(' ')).first();

        QFileInfo info(shell_without_args);
        /*if( info.isRelative() )
        {
            shell_without_args = build_dir + "/" + shell_without_args;
            info.setFile( shell_without_args );
        }*/
        if(!info.exists()) {
            const QString messageText = i18n("Could not locate the debugging shell '%1'.", shell_without_args);
            auto* message = new Sublime::Message(messageText, Sublime::Message::Error);
            KDevelop::ICore::self()->uiController()->postMessage(message);
            return false;
        }

        arguments.insert(0, m_debuggerExecutable);
        arguments.insert(0, shell.toLocalFile());
        m_process->setShellCommand(KShell::joinArgs(arguments));
    } else {
        m_process->setProgram(m_debuggerExecutable, arguments);
        fullCommand = m_debuggerExecutable + QLatin1Char(' ');
    }
    fullCommand += arguments.join(QLatin1Char(' '));

    m_process->start();

    qCDebug(DEBUGGERGDB) << "Starting GDB with command" << fullCommand;
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(5, 78, 0)
    qCDebug(DEBUGGERGDB) << "GDB process pid:" << m_process->pid();
#else
    qCDebug(DEBUGGERGDB) << "GDB process pid:" << m_process->processId();
#endif
    emit userCommandOutput(fullCommand + QLatin1Char('\n'));
    return true;
}
