

/* This file is part of the KDE project
   Copyright (C) 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2002 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2003 Amilcar do Carmo Lucas <amilcar@ida.ing.tu-bs.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include "KDevAppFrontendIface.h"
#include "kdevappfrontend.h"


KDevAppFrontendIface::KDevAppFrontendIface(KDevAppFrontend *appFrontend)
  : DCOPObject("KDevAppFrontend"), m_appFrontend(appFrontend)
{
}


KDevAppFrontendIface::~KDevAppFrontendIface()
{}


void KDevAppFrontendIface::startAppCommand(const QString &directory, const QString &command, bool inTerminal)
{
    m_appFrontend->startAppCommand(directory, command, inTerminal);
}

void KDevAppFrontendIface::stopApplication( )
{
    m_appFrontend->stopApplication();
}

bool KDevAppFrontendIface::isRunning( )
{
    return m_appFrontend->isRunning();
}

void KDevAppFrontendIface::clearView( )
{
    m_appFrontend->clearView();
}

void KDevAppFrontendIface::insertStderrLine( const QString & line )
{
    m_appFrontend->insertStderrLine(line);
}

void KDevAppFrontendIface::insertStdoutLine( const QString & line )
{
    m_appFrontend->insertStdoutLine(line);
}