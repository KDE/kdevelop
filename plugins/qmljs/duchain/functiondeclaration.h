/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef __FUNCTIONDECLARATION_H__
#define __FUNCTIONDECLARATION_H__

#include "duchainexport.h"

#include <language/duchain/functiondeclaration.h>
#include <language/duchain/indexedducontext.h>
#include <language/duchain/duchainregister.h>

namespace QmlJS {

class KDEVQMLJSDUCHAIN_EXPORT FunctionDeclarationData : public KDevelop::FunctionDeclarationData
{
public:
    KDevelop::IndexedDUContext m_prototypeContext;
};

/**
 * @brief Function declaration keeping track of a "prototype" context
 *
 * The prototype of a function can be used, in Javascript, to add methods and
 * members to the objects instantiated by calling the function.
 *
 * The prototype is also used to resolve "this". If a function is assigned to
 * an object member, its "prototype" becomes the internal context of the object.
 * This way, functions assigned to members of the prototype of a class can use
 * "this" to refer to the object on which they are called.
 *
 * @code
 * function Class() { this.name = "Me"; }
 *
 * Class.prototype.print = function() { console.log(this.name) }
 * @endcode
 */
class KDEVQMLJSDUCHAIN_EXPORT FunctionDeclaration : public KDevelop::FunctionDeclaration
{
public:
    FunctionDeclaration(const FunctionDeclaration &rhs);
    FunctionDeclaration(const KDevelop::RangeInRevision &range, KDevelop::DUContext *context);
    explicit FunctionDeclaration(FunctionDeclarationData &data);
    ~FunctionDeclaration() override;

    /**
     * @brief Return the context representing the prototype of this function
     *
     * The returned context, if not null, contains the declarations of the members
     * of the prototype.
     *
     * @note The DUChain must be read-locked
     */
    KDevelop::DUContext* prototypeContext() const;

    /**
     * @brief Set the prototype context of this function
     *
     * @note The DUChain must be write-locked
     */
    void setPrototypeContext(KDevelop::DUContext* context);

    enum {
        Identity = 112
    };

    using Ptr = KDevelop::DUChainPointer<FunctionDeclaration>;

private:
    DUCHAIN_DECLARE_DATA(FunctionDeclaration)
};

}

DUCHAIN_DECLARE_TYPE(QmlJS::FunctionDeclaration)

#endif
