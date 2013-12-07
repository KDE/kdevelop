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

#include <QtCore/QVarLengthArray>

/**
 * Extended QVarLengthArray with additional convenience API.
 */
template<class T, int Prealloc = 256>
class KDevVarLengthArray : public QVarLengthArray<T, Prealloc>
{
    typedef QVarLengthArray<T, Prealloc> Base;

public:
    inline explicit KDevVarLengthArray(int size = 0)
        : Base(size)
    {
    }

    inline KDevVarLengthArray(const KDevVarLengthArray<T, Prealloc> &other)
        : Base(other)
    {
    }

    ///Returns the index of the given item in this array, or -1
    int indexOf(const T& t) const
    {
        const T* const end = Base::end();
        for(const T* it = Base::begin(); it != end; ++it) {
            if(t == *it) {
                return (it - Base::begin());
            }
        }
        return -1;
    }

    ///Returns whether the given item is contained in this array
    inline bool contains(const T& value) const
    {
        return indexOf(value) != -1;
    }

    ///Removes exactly one occurrence of the given value from the array. Returns false if none was found.
    inline bool removeOne(const T& value);

    ///Returns last item in the array.
    inline T& back()
    {
        Q_ASSERT(!Base::isEmpty());
        return *(Base::end()-1);
    }

    ///Returns last item in the array.
    inline const T& back() const
    {
        Q_ASSERT(!Base::isEmpty());
        return *(Base::constEnd()-1);
    }

    ///Removes last item from the array but does not return it, use @c back() if required.
    inline void pop_back()
    {
        Base::removeLast();
    }

    /// @return QList of items in this array
    QList<T> toList() const;

    /// @return QVector of items in this array
    QVector<T> toVector() const;
};

template <class T, int Prealloc>
Q_INLINE_TEMPLATE bool KDevVarLengthArray<T, Prealloc>::removeOne(const T& value)
{
    const int idx = indexOf(value);
    if (idx == -1) {
        return false;
    }
    Base::remove(idx);
    return true;
}

template <class T, int Prealloc>
Q_OUTOFLINE_TEMPLATE QList< T > KDevVarLengthArray<T, Prealloc>::toList() const
{
    QList<T> ret;
    ret.reserve(Base::size());
    const T* const end = Base::constEnd();
    for(const T* it = Base::constBegin(); it != end; ++it) {
        ret << *it;
    }

    return ret;
}

template <class T, int Prealloc>
Q_OUTOFLINE_TEMPLATE QVector< T > KDevVarLengthArray<T, Prealloc>::toVector() const
{
    QVector<T> ret;
    ret.reserve(Base::size());
    const T* const end = Base::constEnd();
    for(const T* it = Base::constBegin(); it != end; ++it) {
        ret << *it;
    }

    return ret;
}

#endif // KDEVPLATFORM_KDEVVARLENGTHARRAY_H
