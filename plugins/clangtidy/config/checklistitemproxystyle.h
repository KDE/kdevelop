/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CLANGTIDY_CHECKLISTITEMPROXYSTYLE_H
#define CLANGTIDY_CHECKLISTITEMPROXYSTYLE_H

// Qt
#include <QProxyStyle>

class QStyleOptionViewItem;

namespace ClangTidy
{

class CheckListItemProxyStyle : public QProxyStyle
{
    Q_OBJECT

public: // QStyle API
    void drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption* option,
                       QPainter* painter, const QWidget* widget = nullptr) const override;

private:
    void drawCheckBox(QPainter* painter, const QStyleOptionViewItem* option) const;
};

}

#endif
