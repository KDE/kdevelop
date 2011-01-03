/*
 * This file is part of KDevelop
 * Copyright 2011 Dmitry Risenberg <dmitry.risenberg@gmail.com>
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

#ifndef MARKER_H
#define MARKER_H

#include <QList>
#include "diff2export.h"

namespace Diff2 {

class DIFF2_EXPORT Marker
{
public:
    enum Type { Start = 0, End = 1 };

public:
    Marker()
    {
        m_type = Marker::Start;
        m_offset = 0;
    }
    Marker( enum Marker::Type type, unsigned int offset )
    {
        m_type = type;
        m_offset = offset;
    }
    ~Marker() {}

public:
    enum Marker::Type type()   const { return m_type;   }
    unsigned int      offset() const { return m_offset; }

    void setType  ( enum Marker::Type type ) { m_type   = type;   }
    void setOffset( unsigned int offset )    { m_offset = offset; }

private:
    enum Marker::Type m_type;
    unsigned int      m_offset;
};

typedef QList<Marker*> MarkerList;
typedef QList<Marker*>::iterator MarkerListIterator;
typedef QList<Marker*>::const_iterator MarkerListConstIterator;

}

#endif // MARKER_H
