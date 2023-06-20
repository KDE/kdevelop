/*
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "isession.h"

namespace KDevelop
{

ISession::ISession( QObject* parent )
    : QObject(parent)
{
}

ISession::~ISession()
{
}

}

#include "moc_isession.cpp"
