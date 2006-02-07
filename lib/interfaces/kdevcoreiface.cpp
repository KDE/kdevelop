/* This file is part of the KDE project
   Copyright (C) 2001 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2001-2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002 Roberto Raggi <roberto@kdevelop.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include <kdebug.h>
#include <dcopclient.h>
#include "kdevcoreiface.h"
#include "kdevcore.h"


KDevCoreIface::KDevCoreIface(KDevCore *core)
    : QObject(core), DCOPObject("KDevCore"), m_core(core)
{
    connect( m_core, SIGNAL(projectOpened()), this, SLOT(forwardProjectOpened()) );
    connect( m_core, SIGNAL(projectClosed()), this, SLOT(forwardProjectClosed()) );
}


KDevCoreIface::~KDevCoreIface()
{}


void KDevCoreIface::forwardProjectOpened()
{
    kDebug(9000) << "dcop emitting project opened" << endl;
    emitDCOPSignal("projectOpened()", QByteArray());
}


void KDevCoreIface::forwardProjectClosed()
{
    kDebug(9000) << "dcop emitting project closed" << endl;
    emitDCOPSignal("projectClosed()", QByteArray());
}

void KDevCoreIface::openProject( const QString & projectFileName )
{
    m_core->openProject(projectFileName);
}

#include "kdevcoreiface.moc"
