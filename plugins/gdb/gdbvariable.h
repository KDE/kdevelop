/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
                const QString& expression, const QString& display = QString());
};

} // end of namespace GDB
} // end of namespace KDevMI

#endif // GDBVARIABLE_H
