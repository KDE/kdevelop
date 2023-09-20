/*
    SPDX-FileCopyrightText: 2015 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KDEVELOP_STACK_H
#define KDEVELOP_STACK_H

#include <QVarLengthArray>

namespace KDevelop {

/**
 * @brief Implementation of a stack based on QVarLengthArray
 *
 * Since stacks are usually short-lived containers, it make sense to optimize their memory usage
 *
 * Internally using QVarLengthArray. The first @p Prealloc items are placed on the stack.
 * If the size of the stack exceeds @p Prealloc, the contents are moved to the heap.
 *
 * @note Make sure to pass a sensible amount for @p Prealloc; avoiding stack overflows
 *
 * The default value for Prealloc, 32,
 * seems to be a good candidate for between conserving stack space and keeping heap allocations low
 * (verified by a few heaptrack runs of duchainify)
 *
 * @sa QVarLengthArray
 */
template<class T, int Prealloc = 32>
class Stack : public QVarLengthArray<T
        , Prealloc>
{
    using Base = QVarLengthArray<T, Prealloc>;

public:
    inline void swap(Stack<T>& other)
    {
        // prevent Stack<->QVarLengthArray swaps
        Base::swap(other);
    }
    inline void push(const T& t)
    {
        Base::append(t);
    }

    inline T pop()
    {
        T r = Base::last();
        Base::removeLast();
        return r;
    }
    inline T& top()
    {
        return Base::last();
    }
    inline const T& top() const
    {
        return Base::last();
    }
};

}

#endif // KDEVELOP_STACK_H
