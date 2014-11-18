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
#include <QStandardItem>

#include "qmakeprojectfile.h"


QMakeFolderItem::QMakeFolderItem( KDevelop::IProject* project, const KDevelop::Path& path,
                                  KDevelop::ProjectBaseItem* parent )
    : KDevelop::ProjectBuildFolderItem( project, path, parent )
{
}

QList<QMakeProjectFile*> QMakeFolderItem::projectFiles() const
{
    return m_projectFiles;
}

void QMakeFolderItem::addProjectFile(QMakeProjectFile* file)
{
    m_projectFiles << file;
}

QMakeFolderItem::~QMakeFolderItem()
{
    qDeleteAll(m_projectFiles);
}

QMakeTargetItem::QMakeTargetItem( QMakeProjectFile* pro, KDevelop::IProject* project,
                                  const QString& s, KDevelop::ProjectBaseItem* parent )
    : KDevelop::ProjectExecutableTargetItem( project, s, parent )
    , m_pro(pro)
{
}

QMakeTargetItem::~QMakeTargetItem()
{
}

QUrl QMakeTargetItem::builtUrl() const
{
    return m_pro->outPwd().append('/' + text());
}

QUrl QMakeTargetItem::installedUrl() const
{
    return QUrl();
}
