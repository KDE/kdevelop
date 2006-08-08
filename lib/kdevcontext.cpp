/* This file is part of KDevelop
Copyright (C) 2001-2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
Copyright (C) 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
Copyright (C) 2001 Sandy Meier <smeier@kdevelop.org>
Copyright (C) 2002 Daniel Engelschalt <daniel.engelschalt@gmx.net>
Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>
Copyright (C) 2003 Harald Fernengel <harry@kdevelop.org>
Copyright (C) 2003,2006 Hamish Rodda <rodda@kde.org>
Copyright (C) 2004 Alexander Dymo <adymo@kdevelop.org>
Copyright (C) 2006 Adam Treat <treat@kde.org>

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

#include "kdevcontext.h"

#include <QDir>

Context::Context()
{}

Context::~Context()
{}

bool Context::hasType( int aType ) const
{
    return aType == this->type();
}

class EditorContext::Private
{
public:
    Private( const KUrl &url, const KTextEditor::Cursor& position, const QString &linestr,
             const QString &wordstr )
            : m_url( url ), m_position( position ),
            m_linestr( linestr ), m_wordstr( wordstr )
    {}

    KUrl m_url;
    KTextEditor::Cursor m_position;
    QString m_linestr, m_wordstr;
};

EditorContext::EditorContext( const KUrl &url, const KTextEditor::Cursor& position,
                              const QString &linestr, const QString &wordstr )
        : Context(), d( new Private( url, position, linestr, wordstr ) )
{}

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

class FileContext::Private
{
public:
    Private( const KUrl::List &urls )
            : m_urls( urls )
    {}

    KUrl::List m_urls;
};

FileContext::FileContext( const KUrl::List &urls )
        : Context(), d( new Private( urls ) )
{}

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

class CodeItemContext::Private
{
public:
    Private( const KDevCodeItem* item ) : m_item( item )
    {}

    const KDevCodeItem* m_item;
};

CodeItemContext::CodeItemContext( const KDevCodeItem* item )
        : Context(), d( new Private( item ) )
{}

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

class ProjectItemContext::Private
{
public:
    Private( const KDevProjectItem* item ) : m_item( item )
    {}

    const KDevProjectItem* m_item;
};

ProjectItemContext::ProjectItemContext( const KDevProjectItem* item )
        : Context(), d( new Private( item ) )
{}

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

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
