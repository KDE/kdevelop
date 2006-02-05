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
#ifndef CODEPROXY_H
#define CODEPROXY_H

#include "kdevcodeproxy.h"

class _FunctionModelItem;

class CodeProxy : public KDevCodeProxy
{
    Q_OBJECT
public:
    CodeProxy( QObject *parent = 0 );
    virtual ~CodeProxy();

    bool filterAcceptsRow( int source_row,
                           const QModelIndex &source_parent ) const;
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;
    void setKindFilter( int kind = -1 );

private:
    bool functionLessThan( const QModelIndex &left,
                           const QModelIndex &right ) const;
private:
    int m_kindFilter;
    QList<int> m_sortKind;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
