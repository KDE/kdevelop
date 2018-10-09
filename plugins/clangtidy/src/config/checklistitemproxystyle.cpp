/*
 * This file is part of KDevelop
 *
 * Copyright 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
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

#include "checklistitemproxystyle.h"

// plugin
#include "checklistmodel.h"
// Qt
#include <QPainter>
#include <QIcon>
#include <QStyleOptionViewItem>

namespace ClangTidy
{

void CheckListItemProxyStyle::drawPrimitive(QStyle::PrimitiveElement element,
                                            const QStyleOption* option, QPainter* painter,
                                            const QWidget* widget) const
{
    Q_UNUSED(widget);
    if (element == QStyle::PE_IndicatorViewItemCheck) {
        auto* optionViewItem = static_cast<const QStyleOptionViewItem*>(option);
        drawCheckBox(painter, optionViewItem);
        return;
    }

    QProxyStyle::drawPrimitive(element, option, painter);
}

void CheckListItemProxyStyle::drawCheckBox(QPainter* painter, const QStyleOptionViewItem* option) const
{
    QString iconName;
    const auto checkState = option->checkState;
    if (checkState == Qt::PartiallyChecked) {
        const int effectiveEnabledState = option->index.data(CheckListModel::EffectiveEnabledStateRole).toInt();
        if (effectiveEnabledState != CheckGroup::Enabled) {
            return;
        }
        iconName = QStringLiteral("dialog-ok");
    } else {
        iconName = (checkState == Qt::Unchecked) ? QStringLiteral("emblem-remove") : QStringLiteral("emblem-added");
    }

    const auto icon = QIcon::fromTheme(iconName);

    const QIcon::Mode iconMode = (option->state & QStyle::State_Enabled) ? QIcon::Normal : QIcon::Disabled;

    icon.paint(painter, option->rect, option->decorationAlignment, iconMode, QIcon::On);
}

}
