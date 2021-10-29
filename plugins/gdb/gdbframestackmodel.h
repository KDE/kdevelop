/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef FRAMESTACKMODEL_H
#define FRAMESTACKMODEL_H

#include "miframestackmodel.h"

namespace KDevMI { namespace GDB {

class DebugSession;
class GdbFrameStackModel : public KDevMI::MIFrameStackModel
{
    Q_OBJECT
public:
    explicit GdbFrameStackModel(DebugSession* session);

    DebugSession* session();
};

} // end of namespace GDB
} // end of namespace KDevMI

#endif // FRAMESTACKMODEL_H
