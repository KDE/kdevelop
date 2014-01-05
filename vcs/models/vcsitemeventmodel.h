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

#ifndef KDEVPLATFORM_VCSITEMEVENTMODEL_H
#define KDEVPLATFORM_VCSITEMEVENTMODEL_H


#include <QStandardItemModel>
#include "../vcsexport.h"

template <typename T> class QList;


namespace KDevelop
{
class VcsItemEvent;

class KDEVPLATFORMVCS_EXPORT VcsItemEventModel : public QStandardItemModel
{
Q_OBJECT
public:
    VcsItemEventModel( QObject* parent );
    ~VcsItemEventModel();
    
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    void addItemEvents( const QList<KDevelop::VcsItemEvent>& );
    KDevelop::VcsItemEvent itemEventForIndex( const QModelIndex& ) const;
};
}

#endif
