/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "breakpointcontroller.h"

#include "debugsession.h"

using namespace KDevMI::LLDB;

BreakpointController::BreakpointController(DebugSession* parent)
    : MIBreakpointController(parent)
{
}

QLatin1String BreakpointController::pendingBreakpointAddress() const
{
    return QLatin1String("0xffffffffffffffff");
}

#include "moc_breakpointcontroller.cpp"
