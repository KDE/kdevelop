/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "idocumentation.h"

// needed by moc include at the end
// for full declaration of (pointer) type of the property provider
#include "idocumentationprovider.h"

namespace KDevelop
{

IDocumentation::IDocumentation()
    : KSharedObject(*(QObject*)this)
{}


IDocumentation::~IDocumentation()
{}

}

#include "moc_idocumentation.cpp"
