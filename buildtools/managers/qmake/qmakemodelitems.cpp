/* KDevelop QMake Support
 *
 * Copyright 2006 Andreas Pakulat <apaku@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "qmakemodelitems.h"

#include <QtCore/QHash>
#include <QtCore/QPair>
#include <QtCore/QList>

#include "qmakeprojectfile.h"


QMakeFolderItem::QMakeFolderItem( KDevelop::IProject* project, QMakeProjectFile* scope,
                                const KUrl& url, QStandardItem* parent )
: KDevelop::ProjectBuildFolderItem( project, url, parent ), m_projectFile( scope )
{
}

QMakeProjectFile* QMakeFolderItem::projectFile() const
{
    return m_projectFile;
}

QMakeFolderItem::~QMakeFolderItem()
{
}

struct QMakeTargetItemPrivate
{
    KUrl::List m_includes;
    QHash<QString, QString> m_env;
    QList<QPair<QString,QString> > m_defs;
};

QMakeTargetItem::QMakeTargetItem( KDevelop::IProject* project, const QString& s,
                                QStandardItem* parent )
    : KDevelop::ProjectTargetItem( project, s, parent ), d(new QMakeTargetItemPrivate)
{
}

QMakeTargetItem::~QMakeTargetItem()
{
    delete d;
}

KUrl::List QMakeTargetItem::includeDirectories() const
{
    return d->m_includes;
}

QHash<QString, QString> QMakeTargetItem::environment() const
{
    return d->m_env;
}

QList<QPair<QString, QString> > QMakeTargetItem::defines() const
{
    return d->m_defs;
}

