/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qttestdelegate.h"


QtTestDelegate::QtTestDelegate(QObject* parent) : QItemDelegate(parent),
passBrush(KColorScheme::View, KColorScheme::PositiveText),
failBrush(KColorScheme::View, KColorScheme::NegativeText),
xFailBrush(KColorScheme::View, KColorScheme::InactiveText),
xPassBrush(KColorScheme::View, KColorScheme::NeutralText),
debugBrush(KColorScheme::View, KColorScheme::NormalText)
{

}

QtTestDelegate::~QtTestDelegate()
{

}

void QtTestDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QString line = index.data().toString();
    QStyleOptionViewItem opt = option;
    if (line.startsWith(QLatin1String("PASS   :")))
    {
        highlight(opt, passBrush);
    }
    else if (line.startsWith(QLatin1String("FAIL!  :")))
    {
        highlight(opt, failBrush);
    }
    else if (line.startsWith(QLatin1String("XFAIL  :")) || line.startsWith(QLatin1String("SKIP   :")))
    {
        highlight(opt, xFailBrush);
    }
    else if (line.startsWith(QLatin1String("XPASS  :")))
    {
        highlight(opt, xPassBrush);
    }
    else if (line.startsWith(QLatin1String("QDEBUG :")))
    {
        highlight(opt, debugBrush);
    }
    QItemDelegate::paint(painter, opt, index);
}

void QtTestDelegate::highlight(QStyleOptionViewItem& option, const KStatefulBrush& brush, bool bold) const
{
    option.font.setBold(bold);
    option.palette.setBrush(QPalette::Text, brush.brush(option.palette));
}

#include "moc_qttestdelegate.cpp"
