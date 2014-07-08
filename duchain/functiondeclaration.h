/*
 * This file is part of qmljs, the QML/JS language support plugin for KDevelop
 * Copyright (c) 2014 Denis Steckelmacher <steckdenis@yahoo.fr>
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
#ifndef __FUNCTIONDECLARATION_H__
#define __FUNCTIONDECLARATION_H__

#include "duchainexport.h"
#include <language/duchain/functiondeclaration.h>
#include <language/duchain/indexedducontext.h>

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
    FunctionDeclaration(FunctionDeclarationData &data);
    ~FunctionDeclaration();

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
     * @param own True if this function declaration must be set as the owner of context
     *
     * @warning This function can be called only one time per function declaration
     * @note The DUChain must be write-locked
     */
    void setPrototypeContext(KDevelop::DUContext* context, bool own);

    enum {
        Identity = 112
    };

    typedef KDevelop::DUChainPointer<FunctionDeclaration> Ptr;
private:
    DUCHAIN_DECLARE_DATA(FunctionDeclaration);
};

}

#endif