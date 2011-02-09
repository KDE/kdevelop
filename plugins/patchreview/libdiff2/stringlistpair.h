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

#ifndef STRINGLISTPAIR_H
#define STRINGLISTPAIR_H

#include <QStringList>
#include "marker.h"

namespace Diff2 {

class StringListPair {
public:
    StringListPair(const QStringList& first, const QStringList& second);
    ~StringListPair();
    bool equal(unsigned int firstIndex, unsigned int secondIndex) const;
    unsigned int lengthFirst() const;
    unsigned int lengthSecond() const;
    MarkerList markerListFirst() const;
    MarkerList markerListSecond() const;
    void prependFirst(Marker* marker);
    void prependSecond(Marker* marker);
    bool needFineGrainedOutput(unsigned int difference) const;

    const static bool allowReplace = false;
private:
    const QStringList m_first;
    const QStringList m_second;
    unsigned int m_lengthFirst;
    unsigned int m_lengthSecond;
    unsigned int* m_hashesFirst;
    unsigned int* m_hashesSecond;
    MarkerList m_markersFirst;
    MarkerList m_markersSecond;
};

}

#endif // STRINGLISTPAIR_H
