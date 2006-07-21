/* This file is part of the KDE project
   Copyright (C) 2001-2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>
   Copyright (C) 2003 Amilcar do Carmo Lucas <amilcar@ida.ing.tu-bs.de>
   Copyright (C) 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
   Copyright (C) 2004 Alexander Dymo <adymo@kdevelop.org>

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
#include "kdevcore.h"
#include <QtDBus/QtDBus>
#include <QDir>

#include "kdevenv.h"

///////////////////////////////////////////////////////////////////////////////
// class Context
///////////////////////////////////////////////////////////////////////////////

Context::Context()
{
}

Context::~Context()
{
}

bool Context::hasType( int aType ) const
{
    return aType == this->type();
}

///////////////////////////////////////////////////////////////////////////////
// class EditorContext
///////////////////////////////////////////////////////////////////////////////

class EditorContext::Private
{
public:
    Private( const KUrl &url, const KTextEditor::Cursor& position, const QString &linestr,
        const QString &wordstr )
        : m_url(url), m_position(position),
          m_linestr(linestr), m_wordstr(wordstr)
    {
    }

    KUrl m_url;
    KTextEditor::Cursor m_position;
    QString m_linestr, m_wordstr;
};

EditorContext::EditorContext( const KUrl &url, const KTextEditor::Cursor& position,
    const QString &linestr, const QString &wordstr )
    : Context(), d( new Private(url, position, linestr, wordstr) )
{
}

EditorContext::~EditorContext()
{
    delete d;
    d = 0;
}

int EditorContext::type() const
{
    return Context::EditorContext;
}

const KUrl &EditorContext::url() const
{
    return d->m_url;
}

const KTextEditor::Cursor& EditorContext::position() const
{
    return d->m_position;
}

QString EditorContext::currentLine() const
{
    return d->m_linestr;
}

QString EditorContext::currentWord() const
{
    return d->m_wordstr;
}

///////////////////////////////////////////////////////////////////////////////
// class FileContext
///////////////////////////////////////////////////////////////////////////////

class FileContext::Private
{
public:
    Private( const KUrl::List &someURLs ) : m_urls(someURLs)
    {
        if (m_urls.count() == 0)
        {
            m_fileName = "INVALID-FILENAME";
            m_isDirectory = false;  // well, "true" should be ok too ...
        }
        else
        {
            m_fileName = m_urls[0].path();
            m_isDirectory = QDir( m_fileName ).exists();
        }
    }
    Private( const QString &fileName, bool isDirectory )
        : m_fileName(fileName), m_isDirectory(isDirectory)
    {
    }

    KUrl::List m_urls;
    /// \FIXME the following data members should be removed, but first other
    // parts should be modified to comply with this change.
    QString m_fileName;
    bool m_isDirectory;
};

FileContext::FileContext( const KUrl::List &someURLs )
    : Context(), d( new Private(someURLs) )
{
}

FileContext::~FileContext()
{
    delete d;
    d = 0;
}

int FileContext::type() const
{
    return Context::FileContext;
}

const KUrl::List &FileContext::urls() const
{
    return d->m_urls;
}

///////////////////////////////////////////////////////////////////////////////
// class DocumentationContext
///////////////////////////////////////////////////////////////////////////////

class DocumentationContext::Private
{
public:
    Private( const QString &url, const QString &selection )
        : m_url(url), m_selection(selection)
    {
    }

    QString m_url;
    QString m_selection;
};

DocumentationContext::DocumentationContext( const QString &url, const QString &selection )
    : Context(), d( new Private(url, selection) )
{
}

DocumentationContext::DocumentationContext( const DocumentationContext &aContext )
    : Context(), d( 0 )
{
    *this = aContext;
}

DocumentationContext &DocumentationContext::operator=( const DocumentationContext &aContext)
{
    if (d) {
        delete d; d = 0;
    }
    d = new Private( *aContext.d );
    return *this;
}

DocumentationContext::~DocumentationContext()
{
    delete d;
    d = 0;
}

int DocumentationContext::type() const
{
    return Context::DocumentationContext;
}

QString DocumentationContext::url() const
{
    return d->m_url;
}

QString DocumentationContext::selection() const
{
    return d->m_selection;
}

///////////////////////////////////////////////////////////////////////////////
// class CodeItemContext
///////////////////////////////////////////////////////////////////////////////

class CodeItemContext::Private
{
public:
    Private( const KDevCodeItem* item ) : m_item( item ) {}

    const KDevCodeItem* m_item;
};

CodeItemContext::CodeItemContext( const KDevCodeItem* item )
    : Context(), d( new Private(item) )
{
}

CodeItemContext::~CodeItemContext()
{
    delete d;
    d = 0;
}

int CodeItemContext::type() const
{
    return Context::CodeItemContext;
}

const KDevCodeItem* CodeItemContext::item() const
{
    return d->m_item;
}

///////////////////////////////////////////////////////////////////////////////
// class ProjectItemContext
///////////////////////////////////////////////////////////////////////////////

class ProjectItemContext::Private
{
public:
    Private( const KDevProjectItem* item ) : m_item( item ) {}

    const KDevProjectItem* m_item;
};

ProjectItemContext::ProjectItemContext( const KDevProjectItem* item )
    : Context(), d( new Private(item) )
{
}

ProjectItemContext::~ProjectItemContext()
{
    delete d;
    d = 0;
}

int ProjectItemContext::type() const
{
    return Context::ProjectItemContext;
}

const KDevProjectItem* ProjectItemContext::item() const
{
    return d->m_item;
}


///////////////////////////////////////////////////////////////////////////////
// class KDevCore
///////////////////////////////////////////////////////////////////////////////

KDevCore::KDevCore( QObject *parent)
    : QObject( parent )
{
    QDBus::sessionBus().registerObject("/org/kdevelop/Core",
                                       this, QDBusConnection::ExportSlots);
}

KDevCore::~KDevCore()
{
}

#include "kdevcore.moc"
