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

//#include <QtCore/qcontainerfwd.h>
#include <QtCore/QtGlobal>
#include <QtCore/QVector>
#include <new>

///Foreach macro that also works with QVarLengthArray or KDevVarLengthArray
///@warning Unlike the Qt foreach macro, this does not temporarily copy the array, which its size must not be changed while the iteration.
#define FOREACH_ARRAY(item, container) for(int a = 0, mustDo = 1; a < container.size(); ++a) if((mustDo == 0 || mustDo == 1) && (mustDo = 2)) for(item(container[a]); mustDo; mustDo = 0)

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Core)

//When this is uncommented, a QVector will be used instead of a variable-length array. This is useful for debugging, to find problems in KDevVarLengthArray
// #define FAKE_KDEVVARLENGTH_ARRAY

#ifdef FAKE_KDEVVARLENGTH_ARRAY
template<class T, int Prealloc = 256>
class KDevVarLengthArray : public QVector<T> {
    public:
    ///Inserts the given item at the given position, moving all items behind the position back
    void insert(const T& item, int position) {
    QVector<T>::insert(position, item);
    }

    // Removes exactly one occurrence of the given value from the array. Returns false if none was found.
    bool removeOne(const T& value) {
    int i = this->indexOf(value);
    if(i == -1)
    return false;
    erase(i);
    return true;
    }
    void erase(int pos) {
    this->remove(pos);
    }
    void append(const T& item) {
        QVector<T>::append(item);
    }
    
    void pop_back() {
        Q_ASSERT(!this->isEmpty());
        QVector<T>::pop_back();
    }
    
    void append(const T *buf, int size) {
    for(int a = 0; a < size; ++a)
        append(buf[a]);
    }
};
#else

template<class T, int Prealloc = 256>
class KDevVarLengthArray
{
public:
    inline explicit KDevVarLengthArray(int size = 0);

    inline KDevVarLengthArray(const KDevVarLengthArray<T, Prealloc> &other)
        : a(Prealloc), s(0), ptr(reinterpret_cast<T *>(array))
    {
        append(other.constData(), other.size());
    }

    inline ~KDevVarLengthArray() {
        if (QTypeInfo<T>::isComplex) {
            T *i = ptr + s;
            while (i-- > ptr)
                i->~T();
        }
        if (ptr != reinterpret_cast<T *>(array))
            qFree(ptr);
    }
    inline KDevVarLengthArray<T, Prealloc> &operator=(const KDevVarLengthArray<T, Prealloc> &other)
    {
        if (this != &other) {
            clear();
            append(other.constData(), other.size());
        }
        return *this;
    }

    inline int size() const { return s; }
    inline int count() const { return s; }
    inline bool isEmpty() const { return (s == 0); }
    inline void resize(int size);
    inline void clear() { resize(0); }

    inline int capacity() const { return a; }
    inline void reserve(int size);

    inline T &operator[](int idx) {
        Q_ASSERT(idx >= 0 && idx < s);
        return ptr[idx];
    }
    inline const T &operator[](int idx) const {
        Q_ASSERT(idx >= 0 && idx < s);
        return ptr[idx];
    }

    ///Returns the index of the given item in this array, or -1
    int indexOf(const T& t) const {
        for(int a = 0; a < s; ++a)
            if(t == ptr[a])
                return a;
        return -1;
    }
    
    inline KDevVarLengthArray& operator<<(const T &t) {
        append(t);
        return *this;
    }

    inline void append(const T &t) {
        const int idx = s++;
        ///This is currently the difference to KDevVarLengthArray(which uses s == a), and it prevents a crash.
        if (s >= a)
            realloc(s, s<<1);
        if (QTypeInfo<T>::isComplex) {
            new (ptr + idx) T(t);
        } else {
            ptr[idx] = t;
        }
    }
    void append(const T *buf, int size);

    inline T *data() { return ptr; }
    inline const T *data() const { return ptr; }
    inline const T * constData() const { return ptr; }

    ///Returns whether the given item is contained in this array
    bool contains(const T& value) const {
      for(int a = 0; a < s; ++a)
          if(ptr[a] == value)
          return true;

      return false;
    }

    ///Inserts the given item at the given position, moving all items behind the position back
    void insert(const T& item, int position) {
        Q_ASSERT(position >= 0 && position <= size());
        resize(s+1);
        for(int a = s-1; a > position; --a) {
            ptr[a] = ptr[a-1];
        }
        ptr[position] = item;
    }

    ///Removes the given position from the array, moving all items behind it one back.
    void erase(int position) {
        Q_ASSERT(position >= 0 && position < s);
        for(int a = position; a < s-1; ++a) {
            ptr[a] = ptr[a+1];
        }
        resize(s-1);
    }

    // Removes exactly one occurrence of the given value from the array. Returns false if none was found.
    bool removeOne(const T& value) {
    for(int a = 0; a < s; ++a) {
        if(ptr[a] == value) {
        erase(a);
        return true;
        }
    }
    return false;
    }

    T& back() {
        return ptr[s-1];
    }

    const T& back() const {
        return ptr[s-1];
    }

    void pop_back() {
        Q_ASSERT(s > 0);
        resize(s-1);
    }

private:
    void realloc(int size, int alloc);

    int a;
    int s;
    T *ptr;
    union {
        // ### Qt 5: Use 'Prealloc * sizeof(T)' as array size
        char array[sizeof(qint64) * (((Prealloc * sizeof(T)) / sizeof(qint64)) + 1)];
        qint64 q_for_alignment_1;
        double q_for_alignment_2;
    };
};

template <class T, int Prealloc>
Q_INLINE_TEMPLATE KDevVarLengthArray<T, Prealloc>::KDevVarLengthArray(int asize)
    : s(asize) {
    if (s > Prealloc) {
        ptr = reinterpret_cast<T *>(qMalloc(s * sizeof(T)));
        a = s;
    } else {
        ptr = reinterpret_cast<T *>(array);
        a = Prealloc;
    }
    if (QTypeInfo<T>::isComplex) {
        T *i = ptr + s;
        while (i != ptr)
            new (--i) T;
    }
}

template <class T, int Prealloc>
Q_INLINE_TEMPLATE void KDevVarLengthArray<T, Prealloc>::resize(int asize)
{ Q_ASSERT(asize >= 0 && asize < 100000); realloc(asize, qMax(asize, a)); }

template <class T, int Prealloc>
Q_INLINE_TEMPLATE void KDevVarLengthArray<T, Prealloc>::reserve(int asize)
{ if (asize > a) realloc(s, asize); }

template <class T, int Prealloc>
Q_OUTOFLINE_TEMPLATE void KDevVarLengthArray<T, Prealloc>::append(const T *abuf, int asize)
{
    Q_ASSERT(abuf);
    if (asize <= 0)
        return;

    const int idx = s;
    const int news = s + asize;
    if (news >= a)
        realloc(news, news<<1);
    else
        s = news;

    if (QTypeInfo<T>::isComplex) {
        T *i = ptr + idx;
        T *j = i + asize;
        while (i < j)
            new (i++) T(*abuf++);
    } else {
        qMemCopy(&ptr[idx], abuf, asize * sizeof(T));
    }
}

template <class T, int Prealloc>
Q_OUTOFLINE_TEMPLATE void KDevVarLengthArray<T, Prealloc>::realloc(int asize, int aalloc)
{
    Q_ASSERT(aalloc >= asize);
    T *oldPtr = ptr;
    int osize = s;
    s = asize;

    if (aalloc != a) {
        ptr = reinterpret_cast<T *>(qMalloc(aalloc * sizeof(T)));
        if (ptr) {
            a = aalloc;

            if (QTypeInfo<T>::isStatic) {
                T *i = ptr + osize;
                T *j = oldPtr + osize;
                while (i != ptr) {
                    new (--i) T(*--j);
                    j->~T();
                }
            } else {
                qMemCopy(ptr, oldPtr, osize * sizeof(T));
            }
        } else {
            ptr = oldPtr;
            s = 0;
            asize = 0;
        }
    }

    if (QTypeInfo<T>::isComplex) {
        if (asize < osize) {
            T *i = oldPtr + osize;
            T *j = oldPtr + asize;
            while (i-- != j)
                i->~T();
        } else {
            T *i = ptr + asize;
            T *j = ptr + osize;
            while (i != j)
                new (--i) T;
        }
    }

    if (oldPtr != reinterpret_cast<T *>(array) && oldPtr != ptr)
        qFree(oldPtr);
}
#endif
QT_END_NAMESPACE

QT_END_HEADER

#endif // QVARLENGTHARRAY_H
