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

#include "helper.h"
#include "functiondeclaration.h"
#include "functiontype.h"
#include "parsesession.h"

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/functiondeclaration.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/namespacealiasdeclaration.h>
#include <language/duchain/types/unsuretype.h>
#include <language/duchain/types/integraltype.h>
#include <language/duchain/types/structuretype.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/typeutils.h>

namespace QmlJS
{
using namespace KDevelop;

AbstractType::Ptr mergeTypes(AbstractType::Ptr type, const AbstractType::Ptr newType)
{
    if (newType && newType->whichType() == AbstractType::TypeFunction) {
        return newType;
    } else {
        return TypeUtils::mergeTypes(type, newType);
    }
}

DeclarationPointer getDeclaration(const QualifiedIdentifier& id, const DUContext* context, bool searchInParent)
{
    DUChainReadLocker lock;
    if (context) {
        auto declarations = context->findDeclarations(
            id.last(),
            CursorInRevision(INT_MAX, INT_MAX),
            nullptr,
            searchInParent ? DUContext::NoSearchFlags : DUContext::DontSearchInParent
        );

        if (declarations.count() > 0) {
            return DeclarationPointer(declarations.last());
        }
    }
    return DeclarationPointer();
}

DeclarationPointer getDeclarationOrSignal(const QualifiedIdentifier& id, const DUContext* context, bool searchInParent)
{
    QString identifier = id.last().toString();

    if (identifier.startsWith(QLatin1String("on")) && identifier.size() > 2) {
        // The use may have typed the name of a QML slot (onFoo), try to get
        // the declaration of its corresponding signal (foo)
        identifier = identifier.at(2).toLower() + identifier.mid(3);
        DeclarationPointer decl = getDeclaration(QualifiedIdentifier(identifier), context, searchInParent);

        if (decl) {
            ClassFunctionDeclaration* classFuncDecl = dynamic_cast<ClassFunctionDeclaration *>(decl.data());

            if (classFuncDecl && classFuncDecl->isSignal()) {
                // Removing "on" has given the identifier of a QML signal, return
                // it instead of the name of its slot
                return decl;
            }
        }
    }

    // No signal found, fall back to normal behavior
    return getDeclaration(id, context, searchInParent);
}

QmlJS::AST::Statement* getQMLAttribute(QmlJS::AST::UiObjectMemberList* members, const QString& attribute)
{
    for (QmlJS::AST::UiObjectMemberList *it = members; it; it = it->next) {
        // The member needs to be a script binding whose name matches attribute
        QmlJS::AST::UiScriptBinding* binding = QmlJS::AST::cast<QmlJS::AST::UiScriptBinding*>(it->member);

        if (binding && binding->qualifiedId && binding->qualifiedId->name == attribute) {
            return binding->statement;
        }
    }

    return NULL;
}

QString getNodeValue(AST::Node* node)
{
    auto identifier = QmlJS::AST::cast<QmlJS::AST::IdentifierExpression*>(node);
    auto identifier_name = QmlJS::AST::cast<QmlJS::AST::IdentifierPropertyName*>(node);
    auto string = QmlJS::AST::cast<QmlJS::AST::StringLiteral*>(node);
    auto string_name = QmlJS::AST::cast<QmlJS::AST::StringLiteralPropertyName*>(node);
    auto true_literal = QmlJS::AST::cast<QmlJS::AST::TrueLiteral*>(node);
    auto false_literal = QmlJS::AST::cast<QmlJS::AST::FalseLiteral*>(node);

    if (identifier) {
        return identifier->name.toString();
    } else if (identifier_name) {
        return identifier_name->id.toString();
    } else if (string) {
        return string->value.toString();
    } else if (string_name) {
        return string_name->id.toString();
    } else if (true_literal) {
        return QLatin1String("true");
    } else if (false_literal) {
        return QLatin1String("false");
    } else {
        return QString();
    }
}

QMLAttributeValue getQMLAttributeValue(QmlJS::AST::UiObjectMemberList* members, const QString& attribute)
{
    QMLAttributeValue res;
    QmlJS::AST::Statement* node = getQMLAttribute(members, attribute);

    // The value of the binding must be an expression
    QmlJS::AST::ExpressionStatement* statement = QmlJS::AST::cast<QmlJS::AST::ExpressionStatement*>(node);

    if (!statement) {
        return res;
    }

    // The expression must be an identifier or a string literal
    res.value = getNodeValue(statement->expression);

    if (res.value.isNull()) {
        return res;
    }

    res.location = statement->expression->firstSourceLocation();

    return res;
}

DUContext* getInternalContext(const DeclarationPointer& declaration)
{
    DUChainReadLocker lock;

    if (!declaration) {
        return nullptr;
    }

    // The internal context of declarations having a function type is the prototype
    // context of the function (if any), or the internal context of Function
    auto functionType = QmlJS::FunctionType::Ptr::dynamicCast(declaration->abstractType());

    if (functionType) {
        Declaration* decl = functionType->declaration(declaration->topContext());
        QmlJS::FunctionDeclaration* funcDecl;

        if (decl && (funcDecl = dynamic_cast<QmlJS::FunctionDeclaration*>(decl)) &&
            funcDecl->prototypeContext()) {
            return funcDecl->prototypeContext();
        } else {
            return getInternalContext(
                getDeclaration(QualifiedIdentifier(QLatin1String("Function")), declaration->topContext())
            );
        }
    }

    // The declaration can either be a class definition (its internal context
    // can be used) or an instance (use the internal context of its type)
    switch (declaration->kind()) {
    case Declaration::Type:
    case Declaration::Namespace:
        return declaration->internalContext();

    case Declaration::NamespaceAlias:
    {
        auto alias = declaration.dynamicCast<NamespaceAliasDeclaration>();

        return getInternalContext(getDeclaration(alias->importIdentifier(), alias->context()));
    }

    default:
    {
        auto structureType = StructureType::Ptr::dynamicCast(declaration->abstractType());
        auto integralType = IntegralType::Ptr::dynamicCast(declaration->abstractType());

        if (structureType) {
            return getInternalContext(
                DeclarationPointer(structureType->declaration(declaration->topContext()))
            );
        } else if (integralType) {
            QString baseClass;

            // Compute from which base Javascript class a type inherits
            switch (integralType->dataType()) {
                case IntegralType::TypeBoolean:
                    baseClass = QLatin1String("Boolean");
                    break;
                case IntegralType::TypeString:
                    baseClass = QLatin1String("String");
                    break;
                case IntegralType::TypeInt:
                case IntegralType::TypeFloat:
                case IntegralType::TypeDouble:
                    baseClass = QLatin1String("Number");
                    break;
                case IntegralType::TypeArray:
                    baseClass = QLatin1String("Array");
                    break;
                default:
                    baseClass = QLatin1String("Object");
                    break;
            }

            return getInternalContext(
                getDeclaration(QualifiedIdentifier(baseClass), declaration->topContext())
            );
        } else {
            return nullptr;
        }
    }
    }
}

DUContext* getInternalFunctionContext(const DeclarationPointer& declaration)
{
    DUChainReadLocker lock;

    if (!declaration) {
        return nullptr;
    }

    auto classFuncDecl = dynamic_cast<ClassFunctionDeclaration*>(declaration.data());
    auto funcDecl = dynamic_cast<FunctionDeclaration*>(declaration.data());

    if (classFuncDecl) {
        return classFuncDecl->internalFunctionContext();
    } else if (funcDecl) {
        return funcDecl->internalFunctionContext();
    } else {
        return nullptr;
    }
}

RangeInRevision emptyRangeOnLine(const AST::SourceLocation& location)
{
    return RangeInRevision(location.startLine - 1, 0, location.startLine - 1, 0);
}

void importDeclarationInContext(DUContext* context, const DeclarationPointer& declaration)
{
    DUContext* importedContext = QmlJS::getInternalContext(declaration);

    if (!importedContext || importedContext == context) {
        return;
    }

    context->addIndirectImport(DUContext::Import(
        importedContext,
        nullptr,                                        // If this is not null, Import::Import will mess with context->owner()->internalFunctionContext...
        CursorInRevision::invalid()
    ));
}

void importObjectContext(DUContext* context, TopDUContext* topContext)
{
    DeclarationPointer objectDeclaration =
        getDeclaration(QualifiedIdentifier(QLatin1String("Object")), topContext);

    if (objectDeclaration) {
        importDeclarationInContext(context, objectDeclaration);
    }
}

bool isPrototypeIdentifier(const QString& identifier)
{
    return (identifier == QLatin1String("prototype") ||
            identifier == QLatin1String("__proto__"));
}

bool isQmlFile(const DUContext* context)
{
    DUChainReadLocker lock;
    return ParseSession::guessLanguageFromSuffix(context->topContext()->url().str()) == Language::Qml;
}

} // End of namespace QmlJS
