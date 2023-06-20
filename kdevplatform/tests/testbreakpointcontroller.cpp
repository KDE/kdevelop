/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "testbreakpointcontroller.h"

using namespace KDevelop;

TestBreakpointController::TestBreakpointController(IDebugSession* parent)
    : IBreakpointController(parent)
{
}

void TestBreakpointController::sendMaybe(Breakpoint* breakpoint)
{
    Q_UNUSED(breakpoint);
}

#include "moc_testbreakpointcontroller.cpp"
