/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    if (element == QStyle::PE_IndicatorItemViewItemCheck) {
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

#include "moc_checklistitemproxystyle.cpp"
