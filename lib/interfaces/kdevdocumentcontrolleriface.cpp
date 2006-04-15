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
#include "kdevdocument.h"

KDevDocumentControllerIface::KDevDocumentControllerIface(KDevDocumentController *pc)
  : QObject(pc), DCOPObject("KDevDocumentController"), m_controller(pc)
{
  connect(pc, SIGNAL(documentLoaded(KDevDocument*)), SLOT(forwardLoadedDocument(KDevDocument*)));
  connect(pc, SIGNAL(documentSaved(KDevDocument*)), SLOT(forwardSavedDocument(KDevDocument*)));
  connect(pc, SIGNAL(documentClosed(KDevDocument*)), SLOT(forwardClosedDocument(KDevDocument*)));
}

KDevDocumentControllerIface::~KDevDocumentControllerIface()
{
}

void KDevDocumentControllerIface::editDocument(const QString &url, int lineNum)
{
  m_controller->editDocument(KUrl(url), KTextEditor::Cursor(lineNum, -1));
}

void KDevDocumentControllerIface::showDocument(const QString &url, bool newWin)
{
  m_controller->showDocumentation(KUrl(url), newWin);
}

void KDevDocumentControllerIface::saveAllDocuments()
{
  m_controller->saveAllDocuments();
}

void KDevDocumentControllerIface::reloadAllDocuments()
{
  m_controller->reloadAllDocuments();
}

void KDevDocumentControllerIface::forwardLoadedDocument(KDevDocument* document)
{
  kDebug(9000) << "dcop emitting loadedDocument " << document->url() << endl;
  emitDCOPSignal("projectOpened()", QByteArray());
}

void KDevDocumentControllerIface::forwardSavedDocument(KDevDocument* document)
{
  kDebug(9000) << "dcop emitting savedDocument " << document->url() << endl;
  emitDCOPSignal("projectClosed()", QByteArray());
}

void KDevDocumentControllerIface::forwardClosedDocument(KDevDocument* document)
{
  kDebug(9000) << "dcop emitting closedDocument " << document->url() << endl;
  emitDCOPSignal("projectClosed()", QByteArray());
}

bool KDevDocumentControllerIface::closeAllDocuments( )
{
  return m_controller->closeAllDocuments();
}

uint KDevDocumentControllerIface::documentState( const KUrl & url )
{
  if (KDevDocument* file = m_controller->documentForUrl(KUrl(url)))
    return (uint)file->state();
  return 0;
}

#include "kdevdocumentcontrolleriface.moc"
