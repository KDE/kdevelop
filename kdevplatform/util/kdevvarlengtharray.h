/*
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2011 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
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
