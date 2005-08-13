/* This file is part of the KDE project
  Copyright (C) 2005 Ian Reinhart Geiser <geiser@kde.org>

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
  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
#include "katedocumentmanagerinterface.h"
#include "kdevpartcontroller.h"
#include <kurl.h>
#include <dcopclient.h>
#include <kapplication.h>
#include <dcopref.h>
#include <kate/document.h>
#include <kurl.h>
#include <kdebug.h>

KateDocumentManagerInterface::KateDocumentManagerInterface( KDevPartController *pc)
    : QObject(pc), DCOPObject("KateDocumentManager"), m_controller(pc)
{

}


KateDocumentManagerInterface::~KateDocumentManagerInterface() {}



DCOPRef KateDocumentManagerInterface::activeDocument( )
{
	return documentWithID(activeDocumentNumber());
}

DCOPRef KateDocumentManagerInterface::document( uint n )
{
  return documentWithID(n);
}

DCOPRef KateDocumentManagerInterface::documentWithID( uint id )
{
  QString dcopobj = "KateDocument#" + QString::number(id);
  return DCOPRef(kapp->dcopClient()->appId(), dcopobj.latin1() );
}

DCOPRef KateDocumentManagerInterface::openURL( const KURL &url, const QString &encoding )
{
  m_controller->editDocument(url);
  m_controller->setEncoding(encoding);
  int idx = findDocument( url );
  return documentWithID(idx);
}

bool KateDocumentManagerInterface::closeAllDocuments( )
{
  return m_controller->closeAllFiles();
}

bool KateDocumentManagerInterface::closeDocument( uint n )
{
  return false;
}

bool KateDocumentManagerInterface::isOpen( const KURL &url )
{
  return (m_controller->partForURL(url) != 0L);
}

int KateDocumentManagerInterface::findDocument( const KURL &url )
{
  KTextEditor::Document *doc = dynamic_cast<KTextEditor::Document*>(m_controller->partForURL(url));
  if( doc )
	return doc->documentNumber();
  else
    return 0L;
}

uint KateDocumentManagerInterface::activeDocumentNumber( )
{
  KTextEditor::Document *doc = dynamic_cast<KTextEditor::Document*>(m_controller->activePart());
  if( doc )
  {
    return doc->documentNumber();
  }
  else
    return 0;
}

uint KateDocumentManagerInterface::documents( )
{
  return m_controller->openURLs().count();
}

#include "katedocumentmanagerinterface.moc"
