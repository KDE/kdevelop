/*
 * LLDB-specific variable controller implementation
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

#include "variablecontroller.h"

#include "debugsession.h"
#include "debuglog.h"
#include "mi/micommand.h"

#include <debugger/variable/variablecollection.h>

using namespace KDevelop;
using namespace KDevMI::LLDB;

VariableController::VariableController(DebugSession *parent)
    : MIVariableController(parent)
{
}

DebugSession *VariableController::debugSession() const
{
    return static_cast<DebugSession*>(const_cast<QObject*>(QObject::parent()));
}

LldbVariable* VariableController::createVariable(TreeModel* model, TreeItem* parent,
                                             const QString& expression, const QString& display)
{
    return new LldbVariable(debugSession(), model, parent, expression, display);
}

void VariableController::update()
{
    qCDebug(DEBUGGERLLDB) << "autoUpdate =" << autoUpdate();
    if (autoUpdate() & UpdateWatches) {
        variableCollection()->watches()->reinstall();
    }

   if (autoUpdate() & UpdateLocals) {
        updateLocals();
   }

   if ((autoUpdate() & UpdateLocals) ||
       ((autoUpdate() & UpdateWatches) && variableCollection()->watches()->childCount() > 0))
    {
        debugSession()->updateAllVariables();
    }
}
