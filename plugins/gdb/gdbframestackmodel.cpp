/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "gdbframestackmodel.h"

#include "debugsession.h"

using namespace KDevMI::GDB;

GdbFrameStackModel::GdbFrameStackModel(DebugSession *session)
    : MIFrameStackModel(session)
{
}

DebugSession* GdbFrameStackModel::session()
{
    return static_cast<DebugSession *>(FrameStackModel::session());
}
