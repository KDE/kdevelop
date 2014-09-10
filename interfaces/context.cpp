/* This file is part of KDevelop
Copyright 2001-2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
Copyright 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
Copyright 2001 Sandy Meier <smeier@kdevelop.org>
Copyright 2002 Daniel Engelschalt <daniel.engelschalt@gmx.net>
Copyright 2002 Simon Hausmann <hausmann@kde.org>
Copyright 2002-2003 Roberto Raggi <roberto@kdevelop.org>
Copyright 2003 Mario Scalas <mario.scalas@libero.it>
Copyright 2003 Harald Fernengel <harry@kdevelop.org>
Copyright 2003,2006 Hamish Rodda <rodda@kde.org>
Copyright 2004 Alexander Dymo <adymo@kdevelop.org>
Copyright 2006 Adam Treat <treat@kde.org>

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

#include "context.h"

#include <QDir>
#include <QList>

#include <ktexteditor/document.h>

namespace KDevelop
{

Context::Context()
    : d(0)
{}

Context::~Context()
{}

bool Context::hasType( int aType ) const
{
    return aType == this->type();
}

class FileContextPrivate
{
public:
    FileContextPrivate( const QList<QUrl> &urls )
            : m_urls( urls )
    {}

    QList<QUrl> m_urls;
};

FileContext::FileContext( const QList<QUrl> &urls )
        : Context(), d( new FileContextPrivate( urls ) )
{}

FileContext::~FileContext()
{
    delete d;
}

int FileContext::type() const
{
    return Context::FileContext;
}

QList<QUrl> FileContext::urls() const
{
    return d->m_urls;
}

class ProjectItemContextPrivate
{
public:
    ProjectItemContextPrivate( const QList<ProjectBaseItem*> &items )
        : m_items( items )
    {}

    QList<ProjectBaseItem*> m_items;
};

ProjectItemContext::ProjectItemContext( const QList<ProjectBaseItem*> &items )
        : Context(), d( new ProjectItemContextPrivate( items ) )
{}

ProjectItemContext::~ProjectItemContext()
{
    delete d;
}

int ProjectItemContext::type() const
{
    return Context::ProjectItemContext;
}

QList<ProjectBaseItem*> ProjectItemContext::items() const
{
    return d->m_items;
}

class OpenWithContextPrivate
{
public:
    OpenWithContextPrivate(const QList<QUrl>& urls, const QMimeType& mimeType)
    : m_urls( urls )
    , m_mimeType( mimeType )
    {}

    QList<QUrl> m_urls;
    QMimeType m_mimeType;
};

OpenWithContext::OpenWithContext(const QList<QUrl>& urls, const QMimeType& mimeType)
: Context()
, d(new OpenWithContextPrivate(urls, mimeType))
{

}

int OpenWithContext::type() const
{
    return Context::OpenWithContext;
}

QList<QUrl> OpenWithContext::urls() const
{
    return d->m_urls;
}

QMimeType OpenWithContext::mimeType() const
{
    return d->m_mimeType;
}

}

