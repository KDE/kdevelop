/*
 * GDB Debugger Support
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

#include "midebuggerplugin.h"
#include "debugsession.h"

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

namespace KDevMI {
namespace GDB
{
class DisassembleWidget;
class GDBOutputWidget;
class MemoryViewerWidget;

class CppDebuggerPlugin : public MIDebuggerPlugin
{
    Q_OBJECT

public:
    friend class DebugSession;

    CppDebuggerPlugin(QObject *parent, const QVariantList & = QVariantList());
    ~CppDebuggerPlugin() override;

    DebugSession *createSession() const override;
    void unload() override;

private:
    void setupToolviews();

private:
    DebuggerToolFactory<DisassembleWidget, CppDebuggerPlugin>* disassemblefactory;
    DebuggerToolFactory<GDBOutputWidget, CppDebuggerPlugin>* gdbfactory;
    DebuggerToolFactory<MemoryViewerWidget, CppDebuggerPlugin>* memoryviewerfactory;
};

} // end of namespace GDB
} // end of namespace KDevMI

#endif
