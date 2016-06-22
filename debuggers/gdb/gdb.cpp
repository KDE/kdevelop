/*
 * Low level GDB interface.
 *
 * Copyright 1999 John Birch <jbb@kdevelop.org >
 * Copyright 2007 Vladimir Prus <ghost@cs.msu.su>
 * Copyright 2016 Aetf <aetf@unlimitedcodeworks.xyz>
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

#include "gdb.h"

#include "dbgglobal.h"
#include "debuglog.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
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
    QUrl gdbUrl = config.readEntry(gdbPathEntry, QUrl());
    if (gdbUrl.isEmpty()) {
        debuggerBinary_ = "gdb";
    } else {
        // FIXME: verify its' a local path.
        debuggerBinary_ = gdbUrl.url(QUrl::PreferLocalFile | QUrl::StripTrailingSlash);
    }

    QStringList arguments = extraArguments;
    arguments << "--interpreter=mi2" << "-quiet";

    QUrl shell = config.readEntry(debuggerShellEntry, QUrl());
    if(!shell.isEmpty()) {
        qCDebug(DEBUGGERGDB) << "have shell" << shell;
        QString shell_without_args = shell.toLocalFile().split(QChar(' ')).first();

        QFileInfo info(shell_without_args);
        /*if( info.isRelative() )
        {
            shell_without_args = build_dir + "/" + shell_without_args;
            info.setFile( shell_without_args );
        }*/
        if(!info.exists()) {
            KMessageBox::information(
                qApp->activeWindow(),
                i18n("Could not locate the debugging shell '%1'.", shell_without_args ),
                i18n("Debugging Shell Not Found") );
            return false;
        }

        arguments.insert(0, debuggerBinary_);
        arguments.insert(0, shell.toLocalFile());
        process_->setShellCommand(KShell::joinArgs(arguments));
    } else {
        process_->setProgram(debuggerBinary_, arguments);
    }

    process_->start();

    qCDebug(DEBUGGERGDB) << "Starting GDB with command" << shell.toLocalFile() + ' ' + debuggerBinary_
                           + ' ' + arguments.join(' ');
    qCDebug(DEBUGGERGDB) << "GDB process pid:" << process_->pid();
    emit userCommandOutput(shell.toLocalFile() + ' ' + debuggerBinary_
                           + ' ' + arguments.join(' ') + '\n');
    return true;
}
