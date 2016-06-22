/*
 * Low level GDB interface.
 *
 * Copyright 2007 Vladimir Prus <ghost@cs.msu.su>
 * Copyright 2016 Aetf <aetf@unlimitedcodeworks.xyz>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef GDB_H_d5c9cb274cbad688fe7a507a84f6633b
#define GDB_H_d5c9cb274cbad688fe7a507a84f6633b

#include "midebugger.h"

namespace KDevMI { namespace GDB {

class GdbDebugger : public MIDebugger
{
    Q_OBJECT
public:
    explicit GdbDebugger(QObject* parent = 0);
    ~GdbDebugger() override;

    bool start(KConfigGroup& config, const QStringList& extraArguments = {}) override;

};

} // end of namespace GDB
} // end of namespace KDevMI

#endif
