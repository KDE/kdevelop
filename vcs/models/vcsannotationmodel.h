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

#ifndef VCSANNOTATIONMODEL_H
#define VCSANNOTATIONMODEL_H

#include <QAbstractTableModel>
#include "../vcsexport.h"

class QModelIndex;
template<typename T> class QList;
class KUrl;

namespace KDevelop
{

class VcsAnnotation;
class VcsAnnotationLine;
    
class KDEVPLATFORMVCS_EXPORT VcsAnnotationModel : public QAbstractTableModel
{
Q_OBJECT
public:
    VcsAnnotationModel( const KUrl& );
    virtual ~VcsAnnotationModel();

    void addLines( const QList<KDevelop::VcsAnnotationLine>& );
    KDevelop::VcsAnnotation annotation() const;

    int rowCount( const QModelIndex& = QModelIndex() ) const;
    int columnCount( const QModelIndex& parent = QModelIndex() ) const;
    QVariant data( const QModelIndex&, int role = Qt::DisplayRole ) const;
    QVariant headerData( int, Qt::Orientation, int role = Qt::DisplayRole ) const;


private:
    class VcsAnnotationModelPrivate* const d;
};

}

#endif
