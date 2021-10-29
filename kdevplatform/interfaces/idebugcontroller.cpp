/*
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "idebugcontroller.h"

namespace KDevelop {

IDebugController::IDebugController(QObject *parent)
    : QObject(parent)
{
}

IDebugController::~IDebugController()
{
}

}

