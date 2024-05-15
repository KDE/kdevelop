/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef GDBBREAKPOINTCONTROLLER_H
#define GDBBREAKPOINTCONTROLLER_H

#include "mibreakpointcontroller.h"

namespace KDevMI { namespace GDB {

class DebugSession;
class BreakpointController : public MIBreakpointController
{
    Q_OBJECT

public:
    explicit BreakpointController(DebugSession *parent);
private:
    QLatin1String pendingBreakpointAddress() const override;
};

} // end of namespace GDB
} // end of namespace KDevMI

#endif // GDBBREAKPOINTCONTROLLER_H
