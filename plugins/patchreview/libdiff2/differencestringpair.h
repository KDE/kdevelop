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

#ifndef DIFFERENCESTRINGPAIR_H
#define DIFFERENCESTRINGPAIR_H

#include <QString>

#include "difference.h"

namespace Diff2 {

class Marker;
class DifferenceString;

class DifferenceStringPair {
public:
    DifferenceStringPair(DifferenceString* first, DifferenceString* second)
        : m_first(first), m_second(second),
        m_strFirst(' ' + first->string()), m_strSecond(' ' + second->string()),
        m_lengthFirst(m_strFirst.length()), m_lengthSecond(m_strSecond.length()),
        m_arrayFirst(m_strFirst.unicode()), m_arraySecond(m_strSecond.unicode())
    {
        // Actual contents must be indented by 1 
    }
    bool equal(unsigned int firstIndex, unsigned int secondIndex) const
    {
        return m_arrayFirst[firstIndex] == m_arraySecond[secondIndex];
    }
    unsigned int lengthFirst() const
    {
        return m_lengthFirst;
    }
    unsigned int lengthSecond() const
    {
        return m_lengthSecond;
    }
    MarkerList markerListFirst() const
    {
        return m_first->markerList();
    }
    MarkerList markerListSecond() const
    {
        return m_second->markerList();
    }
    void prependFirst(Marker* marker)
    {
        m_first->prepend(marker);
    }
    void prependSecond(Marker* marker)
    {
        m_second->prepend(marker);
    }
    bool needFineGrainedOutput(unsigned int difference) const
    {
        return difference <= qMax(m_lengthFirst, m_lengthSecond) / 2;
    }
    const static bool allowReplace = true;
private:
    DifferenceString* m_first;
    DifferenceString* m_second;
    QString m_strFirst;
    QString m_strSecond;
    unsigned int m_lengthFirst;
    unsigned int m_lengthSecond;
    const QChar* m_arrayFirst;
    const QChar* m_arraySecond;
};

}

#endif // DIFFERENCESTRINGPAIR_H