/*************************************************************************************
 *  Copyright (C) 2013 by Andrea Scarpino <scarpino@kde.org>                         *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#ifndef QMLJSDUCHAINHELPERS_H_
#define QMLJSDUCHAINHELPERS_H_

#include <language/duchain/declaration.h>
#include <libs/qmljs/parser/qmljsast_p.h>

#include "duchainexport.h"

namespace QmlJS
{
using namespace KDevelop;

    /**
     * QML attribute value, with its source location
     */
    struct QMLAttributeValue
    {
        QString value;
        AST::SourceLocation location;
    };

    /**
     * Find the declaration for the specified identifier.
     *
     * If no declaration was found, it will return a null pointer.
     *
     * @param id The qualified identifier that identifies our node.
     * @param context A pointer to the DUContext of this node.
     *
     * @note This method already acquires a read lock for the DUChain.
     */
    KDEVQMLJSDUCHAIN_EXPORT DeclarationPointer getDeclaration(const QualifiedIdentifier& id,
                                                              const DUContext* context);

    /**
     * Find an attribute of a QML object
     */
    KDEVQMLJSDUCHAIN_EXPORT AST::Statement* getQMLAttribute(AST::UiObjectMemberList* members,
                                                            const QString &attribute);

    /**
     * Find the string value of a QML attribute
     */
    KDEVQMLJSDUCHAIN_EXPORT QMLAttributeValue getQMLAttributeValue(AST::UiObjectMemberList* members,
                                                                   const QString& attribute);


} // End of namespace QmlJS

#endif /* QMLJSDUCHAINHELPERS_H_ */
