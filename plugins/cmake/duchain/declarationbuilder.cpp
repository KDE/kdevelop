/*
    SPDX-FileCopyrightText: 2014 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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

        if (func.name == QLatin1String("add_executable") || func.name == QLatin1String("add_library")) {
            if (func.arguments.isEmpty()) {
                continue;
            }
            CMakeFunctionArgument arg = func.arguments.first();

            DUChainWriteLocker lock;
            auto* decl = openDeclaration<Declaration>(Identifier(arg.value), arg.range(), DeclarationIsDefinition);
            decl->setAbstractType(AbstractType::Ptr(new TargetType));
            closeDeclaration();
        } else if(func.name == QLatin1String("macro") || func.name == QLatin1String("function")) {
            if (func.arguments.isEmpty()) {
                continue;
            }
            CMakeFunctionArgument arg = func.arguments.first();
            FunctionType::Ptr funcType(new FunctionType);

            auto it=func.arguments.constBegin()+1, itEnd = func.arguments.constEnd();
            for (; it!=itEnd; ++it)
            {
                DelayedType::Ptr delayed(new DelayedType);
                delayed->setIdentifier( IndexedTypeIdentifier(it->value) );
                funcType->addArgument(delayed);
            }

            DUChainWriteLocker lock;
            auto* decl = openDeclaration<FunctionDeclaration>(Identifier(arg.value), arg.range(), DeclarationIsDefinition);
            decl->setAbstractType( funcType );
            closeDeclaration();
        }
    }
}
