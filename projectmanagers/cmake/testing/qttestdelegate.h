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

#ifndef QTTESTDELEGATE_H
#define QTTESTDELEGATE_H

#include <QItemDelegate>
#include <KColorScheme>

class QtTestDelegate : public QItemDelegate
{
    Q_OBJECT
    
public:
    explicit QtTestDelegate(QObject* parent = 0);
    virtual ~QtTestDelegate();
    
    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    
private:
    void highlight(QStyleOptionViewItem& option, const KStatefulBrush& brush, bool bold = true) const;
    
    KStatefulBrush passBrush;
    KStatefulBrush failBrush;
    KStatefulBrush xFailBrush;
    KStatefulBrush debugBrush;
};

#endif // QTTESTDELEGATE_H
