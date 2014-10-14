/* This file is part of KDevelop
    Copyright 2013 Aleix Pol <aleixpol@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "projectmodelitemdelegate.h"

#include "vcsoverlayproxymodel.h"

#include <QPainter>

ProjectModelItemDelegate::ProjectModelItemDelegate(QObject* parent)
    : QItemDelegate(parent)
{}

void ProjectModelItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& opt, const QModelIndex& index) const
{
    QPixmap decoData = decoration(opt, index.data(Qt::DecorationRole));
    QRect decorationRect = rect(opt, index, Qt::DecorationRole);
    QRect checkRect; //unused in practice

    QRect spaceLeft = opt.rect;
    spaceLeft.setLeft(decorationRect.right());
    QString displayData = index.data(Qt::DisplayRole).toString();
    QRect displayRect = textRectangle(painter, spaceLeft, opt.font, displayData);
    displayRect.setLeft(spaceLeft.left());

    QRect branchNameRect(displayRect.topRight(), opt.rect.bottomRight());
    QString branchNameData = index.data(VcsOverlayProxyModel::BranchNameRole).toString();

    doLayout(opt, &checkRect, &decorationRect, &displayRect, false);
    branchNameRect.setLeft(branchNameRect.left() + displayRect.left());
    branchNameRect.setTop(displayRect.top());

    drawStyledBackground(painter, opt);
//     drawCheck(painter, opt, checkRect, checkState);
    drawDecoration(painter, opt, decorationRect, decoData);
    drawDisplay(painter, opt, displayRect, displayData);
    drawBranchName(painter, opt, branchNameRect, branchNameData);
    drawFocus(painter, opt, displayRect);
}

void ProjectModelItemDelegate::drawBranchName(QPainter* painter, const QStyleOptionViewItem& option,
                                              const QRect& rect, const QString& branchName) const
{
    QString text = option.fontMetrics.elidedText(branchName, Qt::ElideRight, rect.width());

    bool selected = option.state & QStyle::State_Selected;
    QPalette::ColorGroup colorGroup = selected ? QPalette::Active : QPalette::Disabled;
    painter->save();
    painter->setPen(option.palette.color(colorGroup, QPalette::Text));
    painter->drawText(rect, text);
    painter->restore();
}

void ProjectModelItemDelegate::drawStyledBackground(QPainter* painter, const QStyleOptionViewItem& option) const
{
    QStyleOptionViewItemV4 opt(option);
    QStyle *style = opt.widget->style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, opt.widget);
}

void ProjectModelItemDelegate::drawDisplay(QPainter* painter, const QStyleOptionViewItem& option,
                                           const QRect& rect, const QString& text) const
{
    QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                            ? QPalette::Normal : QPalette::Disabled;
    if (option.state & QStyle::State_Editing) {
        painter->save();
        painter->setPen(option.palette.color(cg, QPalette::Text));
        painter->drawRect(rect.adjusted(0, 0, -1, -1));
        painter->restore();
    }

    if(text.isEmpty())
        return;

    if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
        cg = QPalette::Inactive;
    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.color(cg, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(cg, QPalette::Text));
    }

    QFontMetrics fm(painter->fontMetrics());
    painter->drawText(rect, fm.elidedText(text, Qt::ElideRight, rect.width()));
}
