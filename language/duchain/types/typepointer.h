/*
 * This file is part of the KDE libraries.
 *
 * Copyright 2005 Frerich Raabe <raabe@kde.org>
 * Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef KDEVPLATFORM_TYPEPOINTER_H
#define KDEVPLATFORM_TYPEPOINTER_H

#include <kdemacros.h>

#include <QtCore/QExplicitlySharedDataPointer>

namespace KDevelop {

/**
 * @brief QExplicitlySharedDataPointer wrapper with convenience functions attached
 */
template<class T>
class TypePtr : public QExplicitlySharedDataPointer<T>
{
    using Base = QExplicitlySharedDataPointer<T>;

public:
    using Base::Base;
    using Base::operator=;

    ///Uses dynamic_cast to cast this pointer to the given type
    template<class U>
    TypePtr<U> cast(U * /*dummy*/ = 0) const {
      return TypePtr<U>(dynamic_cast<U*>(Base::data()));
    }

    /**
     * Convert TypePtr<U> to TypePtr<T>, using a static_cast.
     * This will compile whenever T* and U* are compatible, i.e.
     * T is a subclass of U or vice-versa.
     * Example syntax:
     * <code>
     *   TypePtr<T> tPtr;
     *   TypePtr<U> uPtr = TypePtr<U>::staticCast( tPtr );
     * </code>
     */
    template <class U>
    static TypePtr<T> staticCast( const TypePtr<U>& o ) {
        return TypePtr<T>( static_cast<T *>( o.data() ) );
    }
    /**
     * Convert TypePtr<U> to TypePtr<T>, using a dynamic_cast.
     * This will compile whenever T* and U* are compatible, i.e.
     * T is a subclass of U or vice-versa.
     * Example syntax:
     * <code>
     *   TypePtr<T> tPtr;
     *   TypePtr<U> uPtr = TypePtr<U>::dynamicCast( tPtr );
     * </code>
     * Since a dynamic_cast is used, if U derives from T, and tPtr isn't an instance of U, uPtr will be 0.
     */
    template <class U>
    static TypePtr<T> dynamicCast( const TypePtr<U>& o ) {
        return TypePtr<T>( dynamic_cast<T *>( o.data() ) );
    }
};

}

#endif
