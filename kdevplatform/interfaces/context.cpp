/*
    SPDX-FileCopyrightText: 2001-2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 2001 Sandy Meier <smeier@kdevelop.org>
    SPDX-FileCopyrightText: 2002 Daniel Engelschalt <daniel.engelschalt@gmx.net>
    SPDX-FileCopyrightText: 2002 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2002-2003 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2003 Mario Scalas <mario.scalas@libero.it>
    SPDX-FileCopyrightText: 2003 Harald Fernengel <harry@kdevelop.org>
    SPDX-FileCopyrightText: 2003, 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2004 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "context.h"

#include <QList>
#include <QMimeType>

namespace KDevelop
{

class ContextPrivate
{
public:
    virtual ~ContextPrivate() = default;
};

Context::Context()
    : d_ptr(nullptr)
{}

Context::Context(ContextPrivate* d)
    : d_ptr(d)
{}

Context::~Context() = default;

bool Context::hasType( int aType ) const
{
    return aType == this->type();
}

class FileContextPrivate : public ContextPrivate
{
public:
    explicit FileContextPrivate( const QList<QUrl> &urls )
            : m_urls( urls )
    {}

    QList<QUrl> m_urls;
};

FileContext::FileContext( const QList<QUrl> &urls )
    : Context(new FileContextPrivate(urls))
{}

FileContext::~FileContext() = default;

int FileContext::type() const
{
    return Context::FileContext;
}

QList<QUrl> FileContext::urls() const
{
    Q_D(const FileContext);

    return d->m_urls;
}

class ProjectItemContextPrivate : public ContextPrivate
{
public:
    explicit ProjectItemContextPrivate( const QList<ProjectBaseItem*> &items )
        : m_items( items )
    {}

    QList<ProjectBaseItem*> m_items;
};

ProjectItemContext::ProjectItemContext( const QList<ProjectBaseItem*> &items )
    : Context(new ProjectItemContextPrivate(items))
{}

ProjectItemContext::~ProjectItemContext() = default;

int ProjectItemContext::type() const
{
    return Context::ProjectItemContext;
}

QList<ProjectBaseItem*> ProjectItemContext::items() const
{
    Q_D(const ProjectItemContext);

    return d->m_items;
}

class OpenWithContextPrivate : public ContextPrivate
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
    : Context(new OpenWithContextPrivate(urls, mimeType))
{
}

OpenWithContext::~OpenWithContext() = default;

int OpenWithContext::type() const
{
    return Context::OpenWithContext;
}

QList<QUrl> OpenWithContext::urls() const
{
    Q_D(const OpenWithContext);

    return d->m_urls;
}

QMimeType OpenWithContext::mimeType() const
{
    Q_D(const OpenWithContext);

    return d->m_mimeType;
}

}

