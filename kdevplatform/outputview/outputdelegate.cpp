/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2012 Morten Danielsen Volden <mvolden2@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "outputdelegate.h"

#include "outputmodel.h"
#include "filtereditem.h"

#include <KColorScheme>
#include <KStatefulBrush>

#include <QPainter>


namespace KDevelop
{

class OutputDelegatePrivate
{
public:
    OutputDelegatePrivate();
    QStyleOptionViewItem modifyStyleOptions(const QStyleOptionViewItem& option, const QModelIndex& index) const;

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

QStyleOptionViewItem OutputDelegatePrivate::modifyStyleOptions(const QStyleOptionViewItem& option,
                                                               const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    QVariant status = index.data(OutputModel::OutputItemTypeRole);
    if( status.isValid() ) {
        auto type = static_cast<FilteredItem::FilteredOutputItemType>(status.toInt());
        switch(type) {
            case FilteredItem::ErrorItem:
                opt.palette.setBrush(QPalette::Text, errorBrush.brush(option.palette));
                opt.font.setBold( true );
                break;
            case FilteredItem::WarningItem:
                opt.palette.setBrush(QPalette::Text, warningBrush.brush(option.palette));
                break;
            case FilteredItem::InformationItem:
                opt.palette.setBrush(QPalette::Text, informationBrush.brush(option.palette));
                break;
            case FilteredItem::ActionItem:
                opt.palette.setBrush(QPalette::Text, builtBrush.brush(option.palette));
                opt.font.setBold( true );
                break;
            default:
                break;
        }
    }
    return opt;
}

OutputDelegate::OutputDelegate(QObject* parent)
    : QItemDelegate(parent)
    , d_ptr(new OutputDelegatePrivate)
{
}

OutputDelegate::~OutputDelegate() = default;

void OutputDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_D(const OutputDelegate);

    const QStyleOptionViewItem& opt = d->modifyStyleOptions(option, index);

    QItemDelegate::paint(painter, opt, index);
}

QSize OutputDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_D(const OutputDelegate);

    if (!option.features.testFlag(QStyleOptionViewItem::WrapText)) {
        return QItemDelegate::sizeHint(option, index);
    }

    // Pass item view's contents rect (adjusted) to QItemDelegate::sizeHint() when word wrapping is enabled.
    // QItemDelegate::sizeHint() honors the width of opt.rect but overwrites its height to fit the item's text.
    // When opt.rect is not replaced, enabling word wrapping has no effect: each item remains single-line.

    QStyleOptionViewItem opt = d->modifyStyleOptions(option, index);
    QRect contentsRect = opt.widget->contentsRect();
    // Narrow the rect by the width of a vertical scroll bar to prevent even minuscule horizontal scrolling.
    contentsRect.setWidth(contentsRect.width() - opt.widget->style()->pixelMetric(QStyle::PM_ScrollBarExtent));
    opt.rect = contentsRect;

    return QItemDelegate::sizeHint(opt, index);
}
}

#include "moc_outputdelegate.cpp"
