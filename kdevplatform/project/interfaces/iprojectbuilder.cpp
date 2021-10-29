/*
    SPDX-FileCopyrightText: 2004 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "iprojectbuilder.h"

namespace KDevelop
{
IProjectBuilder::~IProjectBuilder()
{
}


KJob* IProjectBuilder::configure(IProject*)
{
    return nullptr;
}

KJob* IProjectBuilder::prune(IProject*)
{
    return nullptr;
}

QList< IProjectBuilder* > IProjectBuilder::additionalBuilderPlugins( IProject* project ) const
{
    Q_UNUSED( project )
    return QList< IProjectBuilder* >();
}

}

