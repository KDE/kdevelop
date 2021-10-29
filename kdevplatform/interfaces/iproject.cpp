/*
    SPDX-FileCopyrightText: 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2002-2003 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2002 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
    SPDX-FileCopyrightText: 2003 Mario Scalas <mario.scalas@libero.it>
    SPDX-FileCopyrightText: 2003-2004 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "iproject.h"

namespace KDevelop
{

class Path;

IProject::IProject(QObject *parent)
    : QObject(parent)
{
}

IProject::~IProject()
{
}

}

