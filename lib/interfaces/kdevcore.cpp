/***************************************************************************
 *   Copyright (C) 2001-03 by The KDevelop Authors                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "KDevCoreIface.h"
#include "kdevcore.h"

#include "urlutil.h"

///////////////////////////////////////////////////////////////////////////////
// class Context
///////////////////////////////////////////////////////////////////////////////

class Context::Private
{
public:
    Private( const QCString &type ) : m_type(type) {}

    QCString m_type;
};

///////////////////////////////////////////////////////////////////////////////

Context::Context( const QCString &type )
    : d( new Private(type))
{
}

///////////////////////////////////////////////////////////////////////////////

Context::~Context()
{
    delete d;
}

///////////////////////////////////////////////////////////////////////////////

bool Context::hasType( const QCString &type ) const
{
    return type == d->m_type;
}

///////////////////////////////////////////////////////////////////////////////
// class EditorContext
///////////////////////////////////////////////////////////////////////////////

class EditorContext::Private
{
public:
    Private( const KURL &url, int line, int col, const QString &linestr,
        const QString &wordstr )
        : m_url(url), m_line(line), m_col(col),
          m_linestr(linestr), m_wordstr(wordstr)
    {
    }

    KURL m_url;
    int m_line, m_col;
    QString m_linestr, m_wordstr;
};

///////////////////////////////////////////////////////////////////////////////

EditorContext::EditorContext( const KURL &url, int line, int col,
    const QString &linestr, const QString &wordstr )
    : Context("editor"), d( new Private(url, line, col, linestr, wordstr) )
{
}

///////////////////////////////////////////////////////////////////////////////

EditorContext::~EditorContext()
{
    delete d;
}

///////////////////////////////////////////////////////////////////////////////

const KURL &EditorContext::url() const
{
    return d->m_url;
}

///////////////////////////////////////////////////////////////////////////////

int EditorContext::line() const
{
    return d->m_line;
}

///////////////////////////////////////////////////////////////////////////////

int EditorContext::col() const
{
    return d->m_col;
}

///////////////////////////////////////////////////////////////////////////////

QString EditorContext::currentLine() const
{
    return d->m_linestr;
}

///////////////////////////////////////////////////////////////////////////////

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
    Private( const KURL::List &someURLs ) : m_urls(someURLs)
    {
        if (m_urls.count() == 0)
        {
            m_fileName = "INVALID-FILENAME";
            m_isDirectory = false;  // well, "true" should be ok too ...
        }
        else
        {
            m_fileName = m_urls[0].path();
            m_isDirectory = URLUtil::isDirectory( m_urls[0] );
        }
    }
    Private( const QString &fileName, bool isDirectory )
        : m_fileName(fileName), m_isDirectory(isDirectory)
    {
    }

    KURL::List m_urls;
    // FIXME: the following data members should be removed, but first other
    // parts should be modified to comply with this change.
    QString m_fileName;
    bool m_isDirectory;
};

///////////////////////////////////////////////////////////////////////////////

FileContext::FileContext( const KURL::List &someURLs )
    : Context("file"), d( new Private(someURLs) )
{
}

///////////////////////////////////////////////////////////////////////////////

FileContext::FileContext( const QString &fileName, bool isDirectory )
    : Context("file"), d( new Private(fileName, isDirectory))
{
}

///////////////////////////////////////////////////////////////////////////////

FileContext::~FileContext()
{
    delete d;
}

///////////////////////////////////////////////////////////////////////////////

QString FileContext::fileName() const
{
    return d->m_fileName;
}

///////////////////////////////////////////////////////////////////////////////

bool FileContext::isDirectory() const
{
    return d->m_isDirectory;
}

///////////////////////////////////////////////////////////////////////////////

const KURL::List &FileContext::urls() const
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

///////////////////////////////////////////////////////////////////////////////

DocumentationContext::DocumentationContext( const QString &url, const QString &selection )
    : Context("documentation"), d( new Private(url, selection) )
{
}

///////////////////////////////////////////////////////////////////////////////

DocumentationContext::~DocumentationContext()
{
    delete d;
}

///////////////////////////////////////////////////////////////////////////////

QString DocumentationContext::url() const
{
    return d->m_url;
}

///////////////////////////////////////////////////////////////////////////////

QString DocumentationContext::selection() const
{
    return d->m_selection;
}

///////////////////////////////////////////////////////////////////////////////
// class ClassContext
///////////////////////////////////////////////////////////////////////////////

class ClassContext::Private
{
public:
    Private( const QString &classname ) : m_classname(classname) {}

    QString m_classname;
};

///////////////////////////////////////////////////////////////////////////////

ClassContext::ClassContext( const QString &classname )
    : Context("class"), d( new Private( classname ))
{
}

///////////////////////////////////////////////////////////////////////////////

ClassContext::~ClassContext()
{
    delete d;
}

///////////////////////////////////////////////////////////////////////////////

QString ClassContext::classname() const
{
    return d->m_classname;
}

///////////////////////////////////////////////////////////////////////////////
// class KDevCore
///////////////////////////////////////////////////////////////////////////////

KDevCore::KDevCore( QObject *parent, const char *name )
    : QObject( parent, name )
{
    new KDevCoreIface(this);
}

///////////////////////////////////////////////////////////////////////////////

KDevCore::~KDevCore()
{
}


#include "kdevcore.moc"
