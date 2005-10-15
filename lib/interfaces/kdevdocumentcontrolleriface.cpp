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

#include "kdevdocumentcontrolleriface.h"
#include "kdevdocumentcontroller.h"

KDevDocumentControllerIface::KDevDocumentControllerIface(KDevDocumentController *pc)
  : QObject(pc), DCOPObject("KDevDocumentController"), m_controller(pc)
{
  connect(pc, SIGNAL(documentLoaded(const KURL &)), this, SLOT(forwardLoadedDocument(const KURL &)));
  connect(pc, SIGNAL(documentSaved(const KURL &)), this, SLOT(forwardSavedDocument(const KURL &)));
  connect(pc, SIGNAL(documentClosed(const KURL &)), this, SLOT(forwardClosedDocument(const KURL &)));
}

KDevDocumentControllerIface::~KDevDocumentControllerIface()
{
}

void KDevDocumentControllerIface::editDocument(const QString &url, int lineNum)
{
  m_controller->editDocument(KURL(url), lineNum);
}

void KDevDocumentControllerIface::showDocument(const QString &url, bool newWin)
{
  m_controller->showDocument(KURL(url), newWin);
}

void KDevDocumentControllerIface::saveAllDocuments()
{
  m_controller->saveAllDocuments();
}

void KDevDocumentControllerIface::reloadAllDocuments()
{
  m_controller->reloadAllDocuments();
}

void KDevDocumentControllerIface::forwardLoadedDocument(const KURL &url)
{
  kdDebug(9000) << "dcop emitting loadedDocument " << url << endl;
  emitDCOPSignal("projectOpened()", QByteArray());
}

void KDevDocumentControllerIface::forwardSavedDocument(const KURL &url)
{
  kdDebug(9000) << "dcop emitting savedDocument " << url << endl;
  emitDCOPSignal("projectClosed()", QByteArray());
}

void KDevDocumentControllerIface::forwardClosedDocument(const KURL &url)
{
  kdDebug(9000) << "dcop emitting closedDocument " << url << endl;
  emitDCOPSignal("projectClosed()", QByteArray());
}

bool KDevDocumentControllerIface::closeAllDocuments( )
{
  return m_controller->closeAllDocuments();
}

uint KDevDocumentControllerIface::documentState( const KURL & url )
{
  return (uint) m_controller->documentState(url);
}

#include "kdevdocumentcontrolleriface.moc"
