/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>                     *
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

#include "svnoutputdelegate.h"
#include "svnmodels.h"

SvnOutputDelegate::SvnOutputDelegate( QObject* parent )
    : QItemDelegate(parent), conflictBrush( KColorScheme::View, KColorScheme::NegativeText )
{
}

void SvnOutputDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index )
{
    QStyleOptionViewItem opt = option;
    QVariant status = index.data(Qt::UserRole+1);
    if( status.isValid() && status.toInt() == SvnOutputModel::Conflict )
    {
        opt.palette.setBrush( QPalette::Text, conflictBrush.brush( opt.palette ) );
    }
    QItemDelegate::paint(painter, opt, index);
}

