/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef LLDB_VARIABLECONTROLLER_H
#define LLDB_VARIABLECONTROLLER_H

#include "mivariablecontroller.h"
#include "variable.h"

namespace KDevMI { namespace LLDB {

class DebugSession;
class VariableController : public MIVariableController
{
    Q_OBJECT

public:
    explicit VariableController(DebugSession* parent);

    void update() override;
    LldbVariable* createVariable(KDevelop::TreeModel* model, KDevelop::TreeItem* parent,
                                 const QString& expression,
                                 const QString& display = QString()) override;
private:
    DebugSession* debugSession() const;
};

} // end of namespace LLDB
} // end of namespace KDevMI

#endif // LLDB_VARIABLECONTROLLER_H
