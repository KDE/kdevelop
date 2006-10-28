/*
 * This file is part of KDevelop
 *
 * Copyright (c) 2006 Adam Treat <treat@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef CODEDELEGATE_H
#define CODEDELEGATE_H

#include "kdevcodedelegate.h"

class CodeDelegate: public KDevCodeDelegate
{
    Q_OBJECT
public:
    CodeDelegate( QObject *parent = 0 );
    virtual ~CodeDelegate();

    virtual void paint( QPainter *painter, const QStyleOptionViewItem &option,
                        const QModelIndex &index ) const;

    virtual QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;

private:
    void updateStyle( QStyleOptionViewItem&, const QModelIndex& ) const;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
