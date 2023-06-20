/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "gdbvariable.h"

#include "debugsession.h"

using namespace KDevelop;
using namespace KDevMI::GDB;

GdbVariable::GdbVariable(DebugSession *session, TreeModel *model, TreeItem *parent,
                         const QString& expression, const QString& display)
    : MIVariable(session, model, parent, expression, display)
{
}

#include "moc_gdbvariable.cpp"
