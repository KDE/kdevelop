/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef LLDB_BREAKPOINTCONTROLLER_H
#define LLDB_BREAKPOINTCONTROLLER_H

#include "mibreakpointcontroller.h"

namespace KDevMI { namespace LLDB {

class DebugSession;
class BreakpointController : public MIBreakpointController
{
    Q_OBJECT

public:
    explicit BreakpointController(DebugSession *parent);
private:
    QLatin1String pendingBreakpointAddress() const override;
};

} // end of namespace LLDB
} // end of namespace KDevMI

#endif // LLDB_BREAKPOINTCONTROLLER_H
