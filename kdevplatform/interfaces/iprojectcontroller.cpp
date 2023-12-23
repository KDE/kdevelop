/*
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "iprojectcontroller.h"
#include "icore.h"
#include "isession.h"

#include <KConfigGroup>

namespace KDevelop
{

IProjectController::IProjectController( QObject *parent )
        : QObject( parent )
{
}

IProjectController::~IProjectController()
{
}

bool IProjectController::parseAllProjectSources()
{
    KConfigGroup group = ICore::self()->activeSession()->config()->group(QStringLiteral("Project Manager"));
    return group.readEntry( "Parse All Project Sources", true );    
}

}

#include "moc_iprojectcontroller.cpp"
