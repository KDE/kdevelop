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

#include <QtCore/QExplicitlySharedDataPointer>
#include <QtCore/QAtomicPointer>
#include <kdemacros.h>

typedef QSharedData TypeShared;

/**
 * \class TypePtr typepointer.h <TypePtr>
 *
 * Can be used to control the lifetime of an object that has derived
 * QSharedData. As long a someone holds
 * a TypePtr on some QSharedData object it won't become deleted but
 * is deleted once its reference count is 0.
 * This struct emulates C++ pointers virtually perfectly.
 * So just use it like a simple C++ pointer.
 *
 * The difference with using QSharedDataPointer is that QSharedDataPointer is
 * a building block for implementing a value class with implicit sharing (like QString),
 * whereas TypePtr provides refcounting to code that uses pointers.
 *
 * @author Waldo Bastian <bastian@kde.org>
 */
template< class T >
class TypePtr
{
public:
    /**
     * Creates a null pointer.
     */
    inline TypePtr()
        : d(0) { }

    /**
     * Creates a new pointer.
     * @param p the pointer
     */
    inline explicit TypePtr( T* p )
        : d(p) { if(d) d->ref.ref(); }

    /**
     * Copies a pointer.
     * @param o the pointer to copy
     */
    inline TypePtr( const TypePtr& o )
        : d(o.d) { if(d) d->ref.ref(); }

    /**
     * Unreferences the object that this pointer points to. If it was
     * the last reference, the object will be deleted.
     */
    inline ~TypePtr() { if (d && !d->ref.deref()) delete d; }

    inline TypePtr<T>& operator= ( const TypePtr& o ) { attach(o.d); return *this; }
    inline bool operator== ( const TypePtr& o ) const { return ( d == o.d ); }
    inline bool operator!= ( const TypePtr& o ) const { return ( d != o.d ); }
    inline bool operator< ( const TypePtr& o ) const { return ( d < o.d ); }

    inline TypePtr<T>& operator= ( T* p ) { attach(p); return *this; }
    inline bool operator== ( const T* p ) const { return ( d == p ); }
    inline bool operator!= ( const T* p ) const { return ( d != p ); }

    /**
     * Test if the shared pointer is NOT null.
     * @return true if the shared pointer is NOT null, false otherwise.
     * @see isNull
     */
    inline operator bool() const { return ( d != 0 ); }

    /**
     * @return the pointer
     * @warning Since often TypePtr is used only temporarily, it is dangerous to work
     * with the pointer directly, because as soon as the TypePtr gets out of scope,
     * the type instance is deleted. Make sure you have a TypePtr that holds the type alive.
     */
    inline T* unsafeData() { return d; }

    /**
     * @return the pointer
     * @warning Since often TypePtr is used only temporarily, it is dangerous to work
     * with the pointer directly, because as soon as the TypePtr gets out of scope,
     * the type instance is deleted. Make sure you have a TypePtr that holds the type alive.
     */
    inline const T* unsafeData() const { return d; }

    /**
     * @return a const pointer to the shared object.
     */
    inline const T* constData() const { return d; }

    inline const T& operator*() const { return *d; }
    inline T& operator*() { return *d; }
    inline const T* operator->() const { return d; }
    inline T* operator->() { return d; }

    /**
     * Attach the given pointer to the current TypePtr.
     * If the previous shared pointer is not owned by any TypePtr,
     * it is deleted.
     */
    void attach(T* p);

    /**
     * Clear the pointer, i.e. make it a null pointer.
     */
    void clear();

    /**
     * Returns the number of references.
     * @return the number of references
     */
    inline int count() const { return d ? static_cast<int>(d->ref) : 0; } // for debugging purposes

    /**
     * Test if the shared pointer is null.
     * @return true if the pointer is null, false otherwise.
     * @see opertor (bool)
     */
    inline bool isNull() const { return (d == 0); }

    /**
     * @return Whether this is the only shared pointer pointing to
     * to the pointee, or whether it's shared among multiple
     * shared pointers.
     */
    inline bool isUnique() const { return count() == 1; }

    template <class U> friend class TypePtr;

    ///Uses dynamic_cast to cast this pointer to the given type
    template<class U>
    TypePtr<U> cast(U * /*dummy*/ = 0) const {
      return TypePtr<U>(dynamic_cast<U*>(d));
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
        return TypePtr<T>( static_cast<T *>( o.d ) );
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
        return TypePtr<T>( dynamic_cast<T *>( o.d ) );
    }

protected:
    T* d;
};

template <class T>
Q_INLINE_TEMPLATE bool operator== (const T* p, const TypePtr<T>& o)
{
    return ( p == o.d );
}

template <class T>
Q_INLINE_TEMPLATE bool operator!= (const T* p, const TypePtr<T>& o)
{
    return ( p != o.d );
}

template <class T>
Q_INLINE_TEMPLATE void TypePtr<T>::attach(T* p)
{
    if (d != p) {
        if (p) p->ref.ref();
        if (d && !d->ref.deref())
            delete d;
        d = p;
    }
}

template <class T>
Q_INLINE_TEMPLATE void TypePtr<T>::clear()
{
    attach(static_cast<T*>(0));
}

#endif

