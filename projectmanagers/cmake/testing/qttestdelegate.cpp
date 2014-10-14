/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
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
    if (line.startsWith("PASS   :"))
    {
        highlight(opt, passBrush);
    }
    else if (line.startsWith("FAIL!  :"))
    {
        highlight(opt, failBrush);
    }
    else if (line.startsWith("XFAIL  :") || line.startsWith("SKIP   :"))
    {
        highlight(opt, xFailBrush);
    }
    else if (line.startsWith("XPASS  :"))
    {
        highlight(opt, xPassBrush);
    }
    else if (line.startsWith("QDEBUG :"))
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
