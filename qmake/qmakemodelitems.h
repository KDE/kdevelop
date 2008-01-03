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

#ifndef QMAKEMODELITEMS_H
#define QMAKEMODELITEMS_H

#include <projectmodel.h>

class QMakeProjectFile;


class QMakeFolderItem : public KDevelop::ProjectBuildFolderItem
{
public:
    QMakeFolderItem( KDevelop::IProject*, QMakeProjectFile*, const KUrl&, QStandardItem* parent = 0 );
    virtual ~QMakeFolderItem();
    QMakeProjectFile* projectFile() const;

private:
    QMakeProjectFile* m_projectFile;
};

class QMakeTargetItem : public KDevelop::ProjectTargetItem
{
public:
    QMakeTargetItem( KDevelop::IProject*, const QString& s, QStandardItem* parent );
    ~QMakeTargetItem();
    KUrl::List includeDirectories() const;
    QHash<QString, QString> environment() const;
    QList<QPair<QString,QString> > defines() const;
private:
    struct QMakeTargetItemPrivate* const d;
};


#endif

