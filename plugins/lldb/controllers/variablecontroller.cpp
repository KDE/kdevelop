/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

#include "moc_variablecontroller.cpp"
