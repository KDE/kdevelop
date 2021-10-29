/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTBREAKPOINTCONTROLLER_H
#define TESTBREAKPOINTCONTROLLER_H

#include "testsexport.h"

#include <debugger/interfaces/ibreakpointcontroller.h>

namespace KDevelop {
/**
 * Dummy BreakpointController
 */
class KDEVPLATFORMTESTS_EXPORT TestBreakpointController
    : public IBreakpointController
{
    Q_OBJECT

public:
    explicit TestBreakpointController(IDebugSession* parent);

protected:
    void sendMaybe(Breakpoint* breakpoint) override;
};
} // end of namespace KDevelop
#endif // TESTBREAKPOINTCONTROLLER_H
