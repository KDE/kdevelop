/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>                     *
 *   Copyright 2010 Julien Desgats <julien.desgats@gmail.com>              *
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

#ifndef KDEVPLATFORM_PLUGIN_GREPOUTPUTDELEGATE_H
#define KDEVPLATFORM_PLUGIN_GREPOUTPUTDELEGATE_H

#include <QStyledItemDelegate>

class GrepOutputDelegate : public QStyledItemDelegate
{
public:
    explicit GrepOutputDelegate(QObject* parent);
    virtual ~GrepOutputDelegate();

    static GrepOutputDelegate* self();
    void paint(QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const;
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
private:
    static GrepOutputDelegate* m_self;
    QColor blendColor(QColor color1, QColor color2, double blend) const;
};

#endif

