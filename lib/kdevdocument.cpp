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

namespace Koncrete
{

Document::Document(KParts::Part* part, DocumentController* parent)
  : QObject(parent)
  , m_part(part)
{
}

KUrl Document::url() const
{
    return m_url;
}

void Document::setUrl( const KUrl &url )
{
    m_url = url;
}

KParts::Part * Document::part() const
{
  Q_ASSERT(m_part);
  return m_part;
}

KMimeType::Ptr Document::mimeType() const
{
    return m_mimeType;
}

void Document::setMimeType( KMimeType::Ptr mimeType )
{
    m_mimeType = mimeType;
}

DocumentController * Document::parent() const
{
  return static_cast<DocumentController*>(const_cast<QObject*>(QObject::parent()));
}

bool Document::isActive() const
{
  return parent()->activeDocument() == this;
}

bool Document::isInitialized() const
{
    Q_ASSERT(m_part);
    if ( KParts::ReadWritePart *rw = qobject_cast<KParts::ReadWritePart*>( m_part ) )
    {
        return rw->url().isValid();
    }
    return false;
}

bool Document::isReadWrite() const
{
    Q_ASSERT(m_part);
    return (qobject_cast<KParts::ReadWritePart*>( m_part ) != 0L);
}

void Document::save()
{
  parent()->saveDocuments(QList<Document*>() << this);
}

void Document::reload()
{
  parent()->reloadDocument(this);
}

void Document::close()
{
  parent()->closeDocument(this);
}

Document::DocumentState Document::state() const
{
  return m_state;
}

void Document::setState( Document::DocumentState state )
{
    m_state = state;
}

KTextEditor::Document * Document::textDocument() const
{
  Q_ASSERT(m_part);
  return qobject_cast<KTextEditor::Document*>(m_part);
}

void Document::activate()
{
  parent()->activateDocument(this);
}

}

#include "kdevdocument.moc"
