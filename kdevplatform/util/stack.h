/*
 * Copyright 2015  Kevin Funk <kfunk@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
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
    using Base::QVarLengthArray;

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
