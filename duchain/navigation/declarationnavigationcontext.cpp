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
#include "declarationnavigationcontext.h"

#include <QtGui/QTextDocument>  /* For Qt::escape */

#include <language/duchain/classdeclaration.h>
#include <language/duchain/types/structuretype.h>

#include <duchain/functiontype.h>

using namespace KDevelop;

namespace QmlJS {

DeclarationNavigationContext::DeclarationNavigationContext(DeclarationPointer decl,
                                                                  KDevelop::TopDUContextPointer topContext,
                                                                  AbstractNavigationContext* previousContext)
: AbstractDeclarationNavigationContext(decl, topContext, previousContext)
{

}

void DeclarationNavigationContext::htmlIdentifiedType(AbstractType::Ptr type, const IdentifiedType* idType)
{
    ClassDeclaration* classDecl;
    Declaration* decl;

    if ((decl = idType->declaration(m_topContext.data())) &&
        (classDecl = dynamic_cast<ClassDeclaration*>(decl)) &&
        classDecl->qualifiedIdentifier().isEmpty() &&
        classDecl->baseClassesSize() > 0) {
        // If a class declaration has no identifier, display the name of its base
        // class instead of nothing
        BaseClassInstance baseClass = classDecl->baseClasses()[0];

        type = baseClass.baseClass.abstractType();
        idType = TypePtr<StructureType>::dynamicCast(type).constData();
    }

    KDevelop::AbstractDeclarationNavigationContext::htmlIdentifiedType(type, idType);
}

void DeclarationNavigationContext::eventuallyMakeTypeLinks(AbstractType::Ptr type)
{
    auto funcType = QmlJS::FunctionType::Ptr::dynamicCast(type);

    if (type) {
        // Don't let eventuallyMakeTypeLinks cast funcType to an identified type
        // and try to print it! The function most of the time has no name.
        modifyHtml() += typeHighlight(Qt::escape(type->toString()));
    } else {
        KDevelop::AbstractDeclarationNavigationContext::eventuallyMakeTypeLinks(type);
    }
}


}
