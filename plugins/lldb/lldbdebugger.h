/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef LLDBDEBUGGER_H
#define LLDBDEBUGGER_H

#include "midebugger.h"

namespace KDevMI { namespace LLDB {

class LldbDebugger : public MIDebugger
{
    Q_OBJECT
public:
    explicit LldbDebugger(QObject* parent = nullptr);
    ~LldbDebugger() override;

    bool start(KConfigGroup& config, const QStringList& extraArguments = {}) override;

private:
    bool checkVersion();
};

} // end of namespace LLDB
} // end of namespace KDevMI

#endif // LLDBDEBUGGER_H
