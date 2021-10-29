/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "declarationnavigationcontext.h"

#include <QTextDocument>  /* For Qt::escape */

#include <language/duchain/classdeclaration.h>
#include <language/duchain/types/structuretype.h>

#include "../duchain/functiontype.h"

using namespace KDevelop;

namespace QmlJS {

void DeclarationNavigationContext::htmlIdentifiedType(AbstractType::Ptr type, const IdentifiedType* idType)
{
    ClassDeclaration* classDecl;
    Declaration* decl;

    if ((decl = idType->declaration(topContext().data())) &&
        (classDecl = dynamic_cast<ClassDeclaration*>(decl)) &&
        classDecl->qualifiedIdentifier().isEmpty() &&
        classDecl->baseClassesSize() > 0) {
        // If a class declaration has no identifier, display the name of its base
        // class instead of nothing
        BaseClassInstance baseClass = classDecl->baseClasses()[0];

        type = baseClass.baseClass.abstractType();
        idType = TypePtr<StructureType>::dynamicCast(type).constData();
    }

    if (type) {
        KDevelop::AbstractDeclarationNavigationContext::htmlIdentifiedType(type, idType);
    }
}

void DeclarationNavigationContext::eventuallyMakeTypeLinks(AbstractType::Ptr type)
{
    auto funcType = QmlJS::FunctionType::Ptr::dynamicCast(type);

    if (funcType) {
        // Don't let eventuallyMakeTypeLinks cast funcType to an identified type
        // and try to print it! The function most of the time has no name.
        modifyHtml() += typeHighlight(type->toString().toHtmlEscaped());
    } else {
        KDevelop::AbstractDeclarationNavigationContext::eventuallyMakeTypeLinks(type);
    }
}


}
