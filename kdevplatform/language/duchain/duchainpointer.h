/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_DUCHAINPOINTER_H
#define KDEVPLATFORM_DUCHAINPOINTER_H

#include <QMetaType>
#include <QList>
#include <QExplicitlySharedDataPointer>
#include <language/languageexport.h>

//krazy:excludeall=dpointer

namespace KDevelop {
class DUContext;
class TopDUContext;
class DUChainBase;
class Declaration;
class AbstractFunctionDeclaration;

/**
 * Whenever the du-chain is unlocked and locked again, any du-chain item may have been deleted in between.
 * For that reason, the following class should be used to make sure that no deleted objects are accessed. It contains a pointer
 * that will be reset to zero once the pointed object is deleted.
 *
 * Access to the data must still be serialized through duchain-locking. Using this comes with no additional cost.
 *
 * In practice this means:
 * Store an instance of DUChainPointer instead of a pointer to the du-chain object.
 * Then, access the eventually still existing object by calling pointer->base().
 *
 * To make it even more convenient see DUChainPointer
 * */

class KDEVPLATFORMLANGUAGE_EXPORT DUChainPointerData
    : public QSharedData
{
public:
    /**
     * Will return zero once the pointed-to object was deleted
     * */
    DUChainBase* base();

    /**
     * Will return zero once the pointed-to object was deleted
     * */
    DUChainBase* base() const;

    ///Default-initialization of an invalid reference
    DUChainPointerData();

    ~DUChainPointerData();

private:
    ///Should not be used from outside, but is needed sometimes to construct an invalid dummy-pointer
    explicit DUChainPointerData(DUChainBase* base);

    friend class DUChainBase;
    DUChainBase* m_base = nullptr;
    Q_DISABLE_COPY(DUChainPointerData)
};

/**
 * A smart-pointer similar class that conveniently wraps around DUChainPointerData without
 * too many dynamic casts.
 *
 * It can be used like a normal pointer.  In order to cast between pointer types, you should
 * use the staticCast() and dynamicCast() functions as appropriate.
 *
 * Access must be serialized by holding the KDevelop::DUChain::lock() as appropriate for the
 * function(s) being called.
 **/

template <class Type>
class DUChainPointer
{
    template <class OtherType>
    friend class DUChainPointer;

public:
    DUChainPointer() : d(QExplicitlySharedDataPointer<DUChainPointerData>(nullptr))
    {
    }

    DUChainPointer(const DUChainPointer&) = default;
    DUChainPointer(DUChainPointer&&) = default;

    ///This constructor includes dynamic casting. If the object cannot be casted to the type, the constructed DUChainPointer will have value zero.
    template <class OtherType>
    explicit DUChainPointer(OtherType* rhs)
    {
        if (dynamic_cast<Type*>(rhs))
            d = rhs->weakPointer();
    }

    template <class OtherType>
    explicit DUChainPointer(DUChainPointer<OtherType> rhs)
    {
        if (dynamic_cast<Type*>(rhs.data()))
            d = rhs.d;
    }

    explicit DUChainPointer(QExplicitlySharedDataPointer<DUChainPointerData> rhs)
    {
        if (dynamic_cast<Type*>(rhs->base()))
            d = rhs;
    }

    explicit DUChainPointer(Type* rhs)
    {
        if (rhs)
            d = rhs->weakPointer();
    }

    ~DUChainPointer() = default;

    bool operator ==(const DUChainPointer<Type>& rhs) const
    {
        return d.data() == rhs.d.data();
    }

    bool operator !=(const DUChainPointer<Type>& rhs) const
    {
        return d.data() != rhs.d.data();
    }

    ///Returns whether the pointed object is still existing
    operator bool() const {
        return d && d->base();
    }

    Type& operator*() const
    {
        Q_ASSERT(d);
        return *static_cast<Type*>(d->base());
    }

    Type* operator->() const
    {
        Q_ASSERT(d);
        return static_cast<Type*>(d->base());
    }

    bool operator<(const DUChainPointer<Type>& rhs) const
    {
        return d.data() < rhs.d.data();
    }

    template <class NewType>
    DUChainPointer<NewType> dynamicCast() const
    {
        if (d && dynamic_cast<NewType*>(d->base()))  //When the reference to the pointer is constant that doesn't mean that the pointed object needs to be constant
            return DUChainPointer<NewType>(static_cast<NewType*>(d->base()));
        else
            return DUChainPointer<NewType>();
    }

    Type* data() const
    {
        if (!d)
            return nullptr;
        return static_cast<Type*>(d->base());
    }

    DUChainPointer<Type>& operator=(const DUChainPointer<Type>&) = default;
    DUChainPointer<Type>& operator=(DUChainPointer<Type>&&) = default;

    DUChainPointer<Type>& operator=(Type* rhs)
    {
        if (rhs)
            d = rhs->weakPointer();
        else
            d = nullptr;

        return *this;
    }

private:
    QExplicitlySharedDataPointer<DUChainPointerData> d;
};

using DUChainBasePointer = DUChainPointer<DUChainBase>;
using DUContextPointer = DUChainPointer<DUContext>;
using TopDUContextPointer = DUChainPointer<TopDUContext>;
using DeclarationPointer = DUChainPointer<Declaration>;
using FunctionDeclarationPointer = DUChainPointer<AbstractFunctionDeclaration>;
}

Q_DECLARE_METATYPE(KDevelop::DUChainBasePointer)
Q_DECLARE_METATYPE(KDevelop::DeclarationPointer)
Q_DECLARE_METATYPE(KDevelop::DUContextPointer)
Q_DECLARE_METATYPE(KDevelop::TopDUContextPointer)

#endif
