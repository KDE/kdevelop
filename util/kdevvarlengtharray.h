/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.  Alternatively you may (at
** your option) use any later version of the GNU General Public
** License if such license has been publicly approved by Trolltech ASA
** (or its successors, if any) and the KDE Free Qt Foundation. In
** addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.2, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/. If
** you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech, as the sole
** copyright holder for Qt Designer, grants users of the Qt/Eclipse
** Integration plug-in the right for the Qt/Eclipse Integration to
** link to functionality provided by Qt Designer and its related
** libraries.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not expressly
** granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef KDEVVARLENGTHARRAY_H
#define KDEVVARLENGTHARRAY_H

#include <QtCore/QVarLengthArray>

///Foreach macro that also works with QVarLengthArray or KDevVarLengthArray
///@warning Unlike the Qt foreach macro, this does not temporarily copy the array, which means its size must not be changed during the iteration.
#define FOREACH_ARRAY(item, container) \
        for(int a = 0, mustDo = 1; a < container.size(); ++a) \
            if((mustDo == 0 || mustDo == 1) && (mustDo = 2)) \
                for(item(container[a]); mustDo; mustDo = 0)

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
    int indexOf(const T& t) const {
        for(int a = 0; a < Base::size(); ++a) {
            if(t == Base::data()[a]) {
                return a;
            }
        }
        return -1;
    }

    ///Append given item to the array.
    inline KDevVarLengthArray& operator<<(const T &t) {
        Base::append(t);
        return *this;
    }

    ///Returns whether the given item is contained in this array
    inline bool contains(const T& value) const;

    ///Inserts the given item at the given position, moving all items behind the position back
    inline void insert(int position, const T& item);

    ///Removes the given position from the array, moving all items behind it one back.
    inline void remove(int position);

    ///Removes exactly one occurrence of the given value from the array. Returns false if none was found.
    inline bool removeOne(const T& value);

    ///Returns last item in the array.
    inline T& back() {
        return Base::data()[Base::size()-1];
    }

    ///Returns last item in the array.
    inline const T& back() const {
        return Base::data()[Base::size()-1];
    }

    ///Removes last item from the array but does not return it, use @c back() if required.
    inline void pop_back() {
        Q_ASSERT(Base::size() > 0);
        Base::resize(Base::size()-1);
    }

    /// @return QList of items in this array
    QList<T> toList() const;

    /// @return QVector of items in this array
    QVector<T> toVector() const;
};

template <class T, int Prealloc>
Q_INLINE_TEMPLATE bool KDevVarLengthArray<T, Prealloc>::contains(const T& value) const
{
    for(int a = 0; a < Base::size(); ++a) {
        if(Base::data()[a] == value) {
            return true;
        }
    }

    return false;
}

template <class T, int Prealloc>
Q_INLINE_TEMPLATE void KDevVarLengthArray<T, Prealloc>::insert(int position, const T& item)
{
    Q_ASSERT(position >= 0 && position <= Base::size());
    Base::resize(Base::size()+1);
    for(int a = Base::size()-1; a > position; --a) {
        Base::data()[a] = Base::data()[a-1];
    }
    Base::data()[position] = item;
}

template <class T, int Prealloc>
Q_INLINE_TEMPLATE void KDevVarLengthArray<T, Prealloc>::remove(int position)
{
    Q_ASSERT(position >= 0 && position < Base::size());
    for(int a = position; a < Base::size()-1; ++a) {
        Base::data()[a] = Base::data()[a+1];
    }
    Base::resize(Base::size()-1);
}

template <class T, int Prealloc>
Q_INLINE_TEMPLATE bool KDevVarLengthArray<T, Prealloc>::removeOne(const T& value)
{
    for(int a = 0; a < Base::size(); ++a) {
        if(Base::data()[a] == value) {
            remove(a);
            return true;
        }
    }
    return false;
}

template <class T, int Prealloc>
Q_OUTOFLINE_TEMPLATE QList< T > KDevVarLengthArray<T, Prealloc>::toList() const
{
    QList<T> ret;
    for(int a = 0; a < Base::size(); ++a) {
        ret << Base::data()[a];
    }

    return ret;
}

template <class T, int Prealloc>
Q_OUTOFLINE_TEMPLATE QVector< T > KDevVarLengthArray<T, Prealloc>::toVector() const
{
    QVector<T> ret;
    for(int a = 0; a < Base::size(); ++a) {
        ret << Base::data()[a];
    }

    return ret;
}

#endif // KDEVVARLENGTHARRAY_H
