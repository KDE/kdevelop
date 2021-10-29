/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "idocumentation.h"

namespace KDevelop
{

IDocumentation::IDocumentation()
    : KSharedObject(*(QObject*)this)
{}


IDocumentation::~IDocumentation()
{}

}
