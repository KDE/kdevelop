/*
    SPDX-FileCopyrightText: 2007-2008 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "iruncontroller.h"

using namespace KDevelop;

IRunController::IRunController(QObject * parent)
    : KJobTrackerInterface(parent)
{
}

