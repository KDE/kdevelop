/* This file is part of the KDE project
   Copyright (C) 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>

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
#include <kdebug.h>
#include <dcopclient.h>
#include <kurl.h>


#include "KDevPartControllerIface.h"
#include "kdevpartcontroller.h"


KDevPartControllerIface::KDevPartControllerIface(KDevPartController *pc)
  : QObject(pc), DCOPObject("KDevPartController"), m_controller(pc)
{
  connect(pc, SIGNAL(loadedFile(const QString &)), this, SLOT(forwardLoadedFile(const QString &)));
  connect(pc, SIGNAL(savedFile(const QString &)), this, SLOT(forwardSavedFile(const QString &)));
}


KDevPartControllerIface::~KDevPartControllerIface()
{
}


void KDevPartControllerIface::editDocument(const QString &url, int lineNum)
{
  m_controller->editDocument(KURL(url), lineNum);
}


void KDevPartControllerIface::showDocument(const QString &url, bool newWin)
{
  m_controller->showDocument(KURL(url), newWin);
}


void KDevPartControllerIface::saveAllFiles()
{
  m_controller->saveAllFiles();
}


void KDevPartControllerIface::revertAllFiles()
{
  m_controller->revertAllFiles();
}


void KDevPartControllerIface::forwardLoadedFile(const QString &fileName)
{
  kdDebug(9000) << "dcop emitting loadedFile " << fileName << endl;
  emitDCOPSignal("projectOpened()", QByteArray());
}


void KDevPartControllerIface::forwardSavedFile(const QString &fileName)
{
  kdDebug(9000) << "dcop emitting savedFile " << fileName << endl;
  emitDCOPSignal("projectClosed()", QByteArray());
}


#include "KDevPartControllerIface.moc"
