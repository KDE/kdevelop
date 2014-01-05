/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>                     *
 *   Copyright (C) 2012  Morten Danielsen Volden mvolden2@gmail.com        *
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

#include "outputdelegate.h"

#include "outputmodel.h"
#include "filtereditem.h"

#include <QPainter>

#include <kdebug.h>

namespace KDevelop
{

OutputDelegatePrivate::OutputDelegatePrivate()
: errorBrush( KColorScheme::View, KColorScheme::NegativeText )
, warningBrush( KColorScheme::View, KColorScheme::NeutralText )
    //TODO: Maybe ActiveText would be better? Not quite sure...
, informationBrush( KColorScheme::View, KColorScheme::LinkText )
, builtBrush( KColorScheme::View, KColorScheme::PositiveText )
{
}


OutputDelegate::OutputDelegate( QObject* parent )
: QItemDelegate( parent )
, d(new OutputDelegatePrivate)
{
}

OutputDelegate::~OutputDelegate()
{
    delete d;
}

void OutputDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    QStyleOptionViewItem opt = option;
    QVariant status = index.data(OutputModel::OutputItemTypeRole);
    if( status.isValid() ) {
        FilteredItem::FilteredOutputItemType type = static_cast<FilteredItem::FilteredOutputItemType>(status.toInt());
        switch(type) {
            case FilteredItem::ErrorItem:
                opt.palette.setBrush( QPalette::Text, d->errorBrush.brush( option.palette ) );
                opt.font.setBold( true );
                break;
            case FilteredItem::WarningItem:
                opt.palette.setBrush( QPalette::Text, d->warningBrush.brush( option.palette ) );
                break;
            case FilteredItem::InformationItem:
                opt.palette.setBrush( QPalette::Text, d->informationBrush.brush( option.palette ) );
                break;
            case FilteredItem::ActionItem:
                opt.palette.setBrush( QPalette::Text, d->builtBrush.brush( option.palette ) );
                opt.font.setBold( true );
                break;
            default:
                break;
        }
    }
    QItemDelegate::paint(painter, opt, index);
}

}
