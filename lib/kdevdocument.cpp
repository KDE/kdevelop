/* This file is part of the KDE project
   Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#include "kdevdocument.h"

#include <kparts/part.h>
#include <ktexteditor/document.h>

#include "kdevdocumentcontroller.h"

KDevDocument::KDevDocument(KParts::Part* part, KDevDocumentController* parent)
  : QObject(parent)
  , m_part(part)
{
}

KUrl KDevDocument::url() const
{
    return m_url;
}

void KDevDocument::setUrl( const KUrl &url )
{
    m_url = url;
}

KParts::Part * KDevDocument::part() const
{
  Q_ASSERT(m_part);
  return m_part;
}

KMimeType::Ptr KDevDocument::mimeType() const
{
    return m_mimeType;
}

void KDevDocument::setMimeType( KMimeType::Ptr mimeType )
{
    m_mimeType = mimeType;
}

KDevDocumentController * KDevDocument::parent() const
{
  return static_cast<KDevDocumentController*>(const_cast<QObject*>(QObject::parent()));
}

bool KDevDocument::isActive() const
{
  return parent()->activeDocument() == this;
}

bool KDevDocument::isInitialized() const
{
    Q_ASSERT(m_part);
    KParts::ReadWritePart *rw = qobject_cast<KParts::ReadWritePart*>( m_part );
    return rw->url().isValid();
}

bool KDevDocument::isReadWrite() const
{
    Q_ASSERT(m_part);
    return (qobject_cast<KParts::ReadWritePart*>( m_part ) != 0L);
}

void KDevDocument::save()
{
  parent()->saveDocuments(QList<KDevDocument*>() << this);
}

void KDevDocument::reload()
{
  parent()->reloadDocument(this);
}

void KDevDocument::close()
{
  parent()->closeDocument(this);
}

KDevDocument::DocumentState KDevDocument::state() const
{
  return m_state;
}

void KDevDocument::setState( KDevDocument::DocumentState state )
{
    m_state = state;
}

KTextEditor::Document * KDevDocument::textDocument() const
{
  Q_ASSERT(m_part);
  return qobject_cast<KTextEditor::Document*>(m_part);
}

void KDevDocument::activate()
{
  parent()->activateDocument(this);
}

#include "kdevdocument.moc"
