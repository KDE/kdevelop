/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef PROJECTBUILDSETMODEL_H
#define PROJECTBUILDSETMODEL_H

#include <QAbstractTableModel>
#include <QSet>

namespace KDevelop
{
class ProjectBaseItem;
class ICore;
}

class KConfigGroup;

class ProjectBuildSetModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ProjectBuildSetModel( QObject* parent );
    QVariant data( const QModelIndex&, int role = Qt::DisplayRole ) const;
    QVariant headerData( int, Qt::Orientation, int role = Qt::DisplayRole ) const;
    int rowCount( const QModelIndex& = QModelIndex() ) const;
    int columnCount( const QModelIndex&  = QModelIndex() ) const;

    void addProjectItem( KDevelop::ProjectBaseItem* );
    void removeProjectItem( KDevelop::ProjectBaseItem* );
    KDevelop::ProjectBaseItem* itemForIndex( const QModelIndex& );
    QList<KDevelop::ProjectBaseItem*> items();
    void saveSettings( KConfigGroup& ) const;
    void readSettings( KConfigGroup &, KDevelop::ICore* );
private:
    QList<KDevelop::ProjectBaseItem*> m_items;
};

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
