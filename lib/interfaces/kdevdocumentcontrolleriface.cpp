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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include <kdebug.h>
#include <dcopclient.h>
#include <kurl.h>

#include "kdevdocumentcontrolleriface.h"
#include "kdevdocumentcontroller.h"

KDevDocumentControllerIface::KDevDocumentControllerIface(KDevDocumentController *pc)
  : QObject(pc), DCOPObject("KDevDocumentController"), m_controller(pc)
{
  connect(pc, SIGNAL(documentLoaded(const KUrl &)), this, SLOT(forwardLoadedDocument(const KUrl &)));
  connect(pc, SIGNAL(documentSaved(const KUrl &)), this, SLOT(forwardSavedDocument(const KUrl &)));
  connect(pc, SIGNAL(documentClosed(const KUrl &)), this, SLOT(forwardClosedDocument(const KUrl &)));
}

KDevDocumentControllerIface::~KDevDocumentControllerIface()
{
}

void KDevDocumentControllerIface::editDocument(const QString &url, int lineNum)
{
  m_controller->editDocument(KUrl(url), lineNum);
}

void KDevDocumentControllerIface::showDocument(const QString &url, bool newWin)
{
  m_controller->showDocument(KUrl(url), newWin);
}

void KDevDocumentControllerIface::saveAllDocuments()
{
  m_controller->saveAllDocuments();
}

void KDevDocumentControllerIface::reloadAllDocuments()
{
  m_controller->reloadAllDocuments();
}

void KDevDocumentControllerIface::forwardLoadedDocument(const KUrl &url)
{
  kdDebug(9000) << "dcop emitting loadedDocument " << url << endl;
  emitDCOPSignal("projectOpened()", QByteArray());
}

void KDevDocumentControllerIface::forwardSavedDocument(const KUrl &url)
{
  kdDebug(9000) << "dcop emitting savedDocument " << url << endl;
  emitDCOPSignal("projectClosed()", QByteArray());
}

void KDevDocumentControllerIface::forwardClosedDocument(const KUrl &url)
{
  kdDebug(9000) << "dcop emitting closedDocument " << url << endl;
  emitDCOPSignal("projectClosed()", QByteArray());
}

bool KDevDocumentControllerIface::closeAllDocuments( )
{
  return m_controller->closeAllDocuments();
}

uint KDevDocumentControllerIface::documentState( const KUrl & url )
{
  return (uint) m_controller->documentState(url);
}

#include "kdevdocumentcontrolleriface.moc"
