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

#include "stringlistpair.h"

#include <QHash>

using namespace Diff2;

unsigned int StringListPair::lengthFirst() const
{
    return m_lengthFirst;
}

unsigned int StringListPair::lengthSecond() const
{
    return m_lengthSecond;
}

MarkerList StringListPair::markerListFirst() const
{
    return m_markersFirst;
}

MarkerList StringListPair::markerListSecond() const
{
    return m_markersSecond;
}

void StringListPair::prependFirst(Marker* marker)
{
    m_markersFirst.prepend(marker);
}

void StringListPair::prependSecond(Marker* marker)
{
    m_markersSecond.prepend(marker);
}

StringListPair::StringListPair(const QStringList& first, const QStringList& second)
    : m_first(first), m_second(second)
{
    // Do not forget about 1 virtual element - see LevenshteinTable
    m_lengthFirst = first.length() + 1;
    m_lengthSecond = second.length() + 1;

    m_hashesFirst = new unsigned int[m_lengthFirst];
    m_hashesSecond = new unsigned int[m_lengthSecond];

    m_hashesFirst[0] = qHash(QString(""));
    for (unsigned int i = 1; i < m_lengthFirst; ++i) {
        m_hashesFirst[i] = qHash(first[i - 1]);
    }
    m_hashesSecond[0] = qHash(QString(""));
    for (unsigned int i = 1; i < m_lengthSecond; ++i) {
        m_hashesSecond[i] = qHash(second[i - 1]);
    }
}

StringListPair::~StringListPair()
{
    delete[] m_hashesFirst;
    delete[] m_hashesSecond;
}

bool StringListPair::equal(unsigned int firstIndex, unsigned int secondIndex) const
{
    if (m_hashesFirst[firstIndex] != m_hashesSecond[secondIndex]) {
        return false;
    }
    if (firstIndex == 0 || secondIndex == 0) {
        return firstIndex == 0 && secondIndex == 0;
    }
    return m_first[firstIndex - 1] == m_second[secondIndex - 1];
}

bool StringListPair::needFineGrainedOutput(unsigned int) const
{
    return true;
}
