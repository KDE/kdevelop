/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef VARIABLECONTROLLER_H
#define VARIABLECONTROLLER_H

#include "mivariablecontroller.h"

namespace KDevMI { namespace GDB {

class DebugSession;
class VariableController : public MIVariableController
{
    Q_OBJECT

public:
    explicit VariableController(DebugSession* parent);

private:
    DebugSession* debugSession() const;
};

} // end of namespace GDB
} // end of namespace KDevMI

#endif // VARIABLECONTROLLER_H
