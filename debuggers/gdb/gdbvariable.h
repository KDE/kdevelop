/*
 * GDB-specific variable
 * Copyright 2016  Aetf <aetf@unlimitedcodeworks.xyz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef GDBVARIABLE_H
#define GDBVARIABLE_H

#include "mivariable.h"

namespace KDevelop {
class TreeModel;
class TreeItem;
}

namespace KDevMI { namespace GDB {
class DebugSession;
class GdbVariable : public KDevMI::MIVariable
{
    Q_OBJECT

public:
    GdbVariable(DebugSession *session, KDevelop::TreeModel* model, KDevelop::TreeItem* parent,
                const QString& expression, const QString& display = "");
};

} // end of namespace GDB
} // end of namespace KDevMI

#endif // GDBVARIABLE_H
