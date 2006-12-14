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

#include "kdevprojectmodel.h"

class QMakeProjectScope;

class QMakeFolderItem : public KDevProjectFolderItem
{
public:
    QMakeFolderItem( QMakeProjectScope*, const KUrl&, QStandardItem* parent = 0 );
    ~QMakeFolderItem();
    QMakeProjectScope* projectScope() const;

private:
    QMakeProjectScope* m_projectScope;
};

class QMakeTargetItem : public KDevProjectTargetItem
{
  public:
    QMakeTargetItem( const QString& s, QStandardItem* parent );
    ~QMakeTargetItem();
    const KUrl::List& includeDirectories() const;
    const QHash<QString, QString>& environment() const;
    const DomUtil::PairList& defines() const;
    private:
        KUrl::List m_includes;
        QHash<QString, QString> m_env;
        DomUtil::PairList m_defs;
};


#endif

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;
