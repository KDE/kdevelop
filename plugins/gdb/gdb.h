/*
    SPDX-FileCopyrightText: 2007 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GDB_H_d5c9cb274cbad688fe7a507a84f6633b
#define GDB_H_d5c9cb274cbad688fe7a507a84f6633b

#include "midebugger.h"

namespace KDevMI { namespace GDB {

class GdbDebugger : public MIDebugger
{
    Q_OBJECT
public:
    explicit GdbDebugger(QObject* parent = nullptr);
    ~GdbDebugger() override;

    bool start(KConfigGroup& config, const QStringList& extraArguments = {}) override;
};

} // end of namespace GDB
} // end of namespace KDevMI

#endif
