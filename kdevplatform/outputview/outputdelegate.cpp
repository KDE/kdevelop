/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2012 Morten Danielsen Volden <mvolden2@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "outputdelegate.h"

#include "outputmodel.h"
#include "filtereditem.h"

#include <KColorScheme>

#include <QPainter>


namespace KDevelop
{

class OutputDelegatePrivate
{
public:
    OutputDelegatePrivate();

    KStatefulBrush errorBrush;
    KStatefulBrush warningBrush;
    KStatefulBrush informationBrush;
    KStatefulBrush builtBrush;
};


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
, d_ptr(new OutputDelegatePrivate)
{
}

OutputDelegate::~OutputDelegate() = default;

void OutputDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    Q_D(const OutputDelegate);

    QStyleOptionViewItem opt = option;
    QVariant status = index.data(OutputModel::OutputItemTypeRole);
    if( status.isValid() ) {
        auto type = static_cast<FilteredItem::FilteredOutputItemType>(status.toInt());
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
