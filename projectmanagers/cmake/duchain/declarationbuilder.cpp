/* KDevelop CMake Support
 *
 * Copyright 2014 Aleix Pol <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "declarationbuilder.h"
#include <cmakeduchaintypes.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/delayedtype.h>

using namespace KDevelop;

void DeclarationBuilder::startVisiting(CMakeContentIterator* node)
{
    for(; node->hasNext(); ) {
        const CMakeFunctionDesc& func = node->next();

        if (func.name == "add_executable" || func.name == "add_library") {
            CMakeFunctionArgument arg = func.arguments.first();

            DUChainWriteLocker lock;
            Declaration* decl = openDeclaration<Declaration>(QualifiedIdentifier(arg.value), arg.range(), DeclarationIsDefinition);
            decl->setAbstractType(AbstractType::Ptr(new TargetType));
            closeDeclaration();
        } else if(func.name == "macro" || func.name == "function") {
            CMakeFunctionArgument arg = func.arguments.first();
            FunctionType::Ptr funcType(new FunctionType);

            QList<CMakeFunctionArgument>::const_iterator it=func.arguments.constBegin()+1, itEnd = func.arguments.constEnd();
            for (; it!=itEnd; ++it)
            {
                DelayedType::Ptr delayed(new DelayedType);
                delayed->setIdentifier( IndexedTypeIdentifier(it->value) );
                funcType->addArgument(delayed);
            }

            DUChainWriteLocker lock;
            FunctionDeclaration* decl = openDeclaration<FunctionDeclaration>(QualifiedIdentifier(arg.value), arg.range(), DeclarationIsDefinition);
            decl->setAbstractType( funcType );
            closeDeclaration();
        }
    }
}
