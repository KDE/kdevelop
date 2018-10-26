/*
    Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    Copyright 2011 Milian Wolff <mail@milianw.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#ifndef KDEVPLATFORM_KDEVVARLENGTHARRAY_H
#define KDEVPLATFORM_KDEVVARLENGTHARRAY_H

#include <QVarLengthArray>
#include <QVector>

/**
 * Extended QVarLengthArray with additional convenience API.
 */
template<class T, int Prealloc = 256>
class KDevVarLengthArray : public QVarLengthArray<T
        , Prealloc>
{
    using Base = QVarLengthArray<T, Prealloc>;

public:
    using Base::QVarLengthArray;

    ///Removes exactly one occurrence of the given value from the array. Returns false if none was found.
    inline bool removeOne(const T& value);

    /// @return QList of items in this array
    QList<T> toList() const;

    /// @return QVector of items in this array
    QVector<T> toVector() const;
};

template<class T, int Prealloc>
Q_INLINE_TEMPLATE bool KDevVarLengthArray<T, Prealloc>::removeOne(const T& value)
{
    const int idx = Base::indexOf(value);
    if (idx == -1) {
        return false;
    }
    Base::remove(idx);
    return true;
}

template<class T, int Prealloc>
Q_OUTOFLINE_TEMPLATE QList<T> KDevVarLengthArray<T, Prealloc>::toList() const
{
    QList<T> ret;
    ret.reserve(Base::size());
    const T* const end = Base::constEnd();
    for (const T* it = Base::constBegin(); it != end; ++it) {
        ret << *it;
    }

    return ret;
}

template<class T, int Prealloc>
Q_OUTOFLINE_TEMPLATE QVector<T> KDevVarLengthArray<T, Prealloc>::toVector() const
{
    QVector<T> ret;
    ret.reserve(Base::size());
    const T* const end = Base::constEnd();
    for (const T* it = Base::constBegin(); it != end; ++it) {
        ret << *it;
    }

    return ret;
}

#endif // KDEVPLATFORM_KDEVVARLENGTHARRAY_H
