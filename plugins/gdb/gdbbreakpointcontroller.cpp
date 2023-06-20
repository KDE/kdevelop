/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "gdbbreakpointcontroller.h"

#include "debugsession.h"

using namespace KDevMI::GDB;

BreakpointController::BreakpointController(DebugSession* parent)
    : MIBreakpointController(parent)
{
}

#include "moc_gdbbreakpointcontroller.cpp"
