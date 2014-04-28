/*************************************************************************************
 *  Copyright (C) 2012 by Aleix Pol <aleixpol@kde.org>                               *
 *  Copyright (C) 2012 by Milian Wolff <mail@milianw.de>                             *
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

#include "declarationbuilder.h"

#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/integraltype.h>
#include <language/duchain/types/typeutils.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/classdeclaration.h>

#include "expressionvisitor.h"
#include "parsesession.h"
#include "helper.h"

using namespace KDevelop;

DeclarationBuilder::DeclarationBuilder(ParseSession* session)
{
    m_session = session;
}

ReferencedTopDUContext DeclarationBuilder::build(const IndexedString& url,
                                                 QmlJS::AST::Node* node,
                                                 ReferencedTopDUContext updateContext)
{
    ///TODO: cleanup
    Q_ASSERT(m_session->url() == url);
    return DeclarationBuilderBase::build(url, node, updateContext);
}

/*
 * Functions
 */
void DeclarationBuilder::visitFunction(QmlJS::AST::FunctionExpression* node)
{
    setComment(m_session->commentForLocation(node->firstSourceLocation()).toUtf8());

    FunctionType::Ptr func(new FunctionType);
    QualifiedIdentifier name;
    RangeInRevision range;

    if (node->kind == QmlJS::AST::Node::Kind_FunctionDeclaration) {
        // Only function declarations have an identifier. Expressions are anonymous
        name = QualifiedIdentifier(node->name.toString());
        range = m_session->locationToRange(node->identifierToken);
    }

    {
        DUChainWriteLocker lock;
        FunctionDeclaration *decl = openDeclaration<FunctionDeclaration>(name, range);
        decl->setKind(Declaration::Type);
    }
    openType(func);
}

bool DeclarationBuilder::visit(QmlJS::AST::FunctionDeclaration* node)
{
    visitFunction(node);

    return DeclarationBuilderBase::visit(node);
}

bool DeclarationBuilder::visit(QmlJS::AST::FunctionExpression* node)
{
    visitFunction(node);

    return DeclarationBuilderBase::visit(node);
}

bool DeclarationBuilder::visit(QmlJS::AST::FormalParameterList* node)
{
    for (QmlJS::AST::FormalParameterList *plist = node; plist; plist = plist->next) {
        const QualifiedIdentifier name(plist->name.toString());
        const RangeInRevision range = m_session->locationToRange(plist->identifierToken);

        DUChainWriteLocker lock;
        Declaration* dec = openDeclaration<Declaration>(name, range);
        IntegralType::Ptr type(new IntegralType(IntegralType::TypeMixed));
        dec->setType(type);
        closeDeclaration();

        if (FunctionType::Ptr funType = currentType<FunctionType>()) {
            funType->addArgument(type.cast<AbstractType>());
        }
    }

    return DeclarationBuilderBase::visit(node);
}

bool DeclarationBuilder::visit(QmlJS::AST::ReturnStatement* node)
{
    if (FunctionType::Ptr func = currentType<FunctionType>()) {
        AbstractType::Ptr returnType;

        if (node->expression) {
            returnType = findType(node->expression).type;
        } else {
            returnType = new IntegralType(IntegralType::TypeVoid);
        }

        DUChainWriteLocker lock;

        func->setReturnType(TypeUtils::mergeTypes(func->returnType(), returnType));
    }

    return false;   // findType has already explored node
}

void DeclarationBuilder::endVisitFunction(QmlJS::AST::FunctionExpression*)
{
    FunctionType::Ptr func = currentType<FunctionType>();

    if (func && !func->returnType()) {
        // A function that returns nothing returns void
        DUChainWriteLocker lock;

        func->setReturnType(AbstractType::Ptr(new IntegralType(IntegralType::TypeVoid)));
    }

    closeAndAssignType();
}

void DeclarationBuilder::endVisit(QmlJS::AST::FunctionDeclaration* node)
{
    DeclarationBuilderBase::endVisit(node);

    endVisitFunction(node);
}

void DeclarationBuilder::endVisit(QmlJS::AST::FunctionExpression* node)
{
    DeclarationBuilderBase::endVisit(node);

    endVisitFunction(node);
}

/*
 * Variables
 */
bool DeclarationBuilder::visit(QmlJS::AST::VariableDeclaration* node)
{
    setComment(m_session->commentForLocation(node->firstSourceLocation()).toUtf8());

    const QualifiedIdentifier name(node->name.toString());
    const RangeInRevision range = m_session->locationToRange(node->identifierToken);
    const AbstractType::Ptr type = findType(node->expression).type;

    {
        DUChainWriteLocker lock;
        openDeclaration<Declaration>(name, range);
    }
    openType(type);

    return false;   // findType has already explored node
}

void DeclarationBuilder::endVisit(QmlJS::AST::VariableDeclaration* node)
{
    DeclarationBuilderBase::endVisit(node);

    closeAndAssignType();
}

bool DeclarationBuilder::visit(QmlJS::AST::BinaryExpression* node)
{
    if (node->op == QSOperator::Assign) {
        ContextBuilder::ExpressionType leftType = findType(node->left);
        AbstractType::Ptr rightType = findType(node->right).type;

        if (leftType.declaration) {
            // Merge the already-known type of the variable with the new one
            DUChainWriteLocker lock;

            leftType.declaration->setAbstractType(TypeUtils::mergeTypes(leftType.type, rightType));
        }

        return false;   // findType has already explored node
    }

    return DeclarationBuilderBase::visit(node);
}

bool DeclarationBuilder::visit(QmlJS::AST::CallExpression* node)
{
    ContextBuilder::ExpressionType expr = findType(node->base);
    FunctionType::Ptr func_type = FunctionType::Ptr::dynamicCast(expr.type);
    DUChainWriteLocker lock;

    if (!expr.declaration || !func_type) {
        return DeclarationBuilderBase::visit(node);
    }

    auto func_declaration = expr.declaration.dynamicCast<FunctionDeclaration>();

    if (!func_declaration) {
        return DeclarationBuilderBase::visit(node);
    }

    // Put the argument nodes in a list that has a definite size
    QVector<Declaration *> arguments_decls = func_declaration->internalFunctionContext()->localDeclarations();
    QVector<QmlJS::AST::ArgumentList *> arguments;

    for (auto argument = node->arguments; argument; argument = argument->next) {
        arguments.append(argument);
    }

    // Don't update a function when it is called with the wrong number
    // of arguments
    if (arguments.size() != arguments_decls.size()) {
        return DeclarationBuilderBase::visit(node);
    }

    // Update the types of the function arguments
    FunctionType::Ptr new_func_type(new FunctionType);

    for (int i=0; i<arguments.size(); ++i) {
        QmlJS::AST::ArgumentList *argument = arguments.at(i);
        Declaration *current_declaration = arguments_decls.at(i);
        AbstractType::Ptr current_type = current_declaration->abstractType();

        // Merge the current type of the argument with its type in the call expression
        AbstractType::Ptr call_type = findType(argument->expression).type;
        AbstractType::Ptr new_type = TypeUtils::mergeTypes(current_type, call_type);

        // Update the declaration of the argument and its type in the function type
        current_declaration->setAbstractType(new_type);
        new_func_type->addArgument(new_type);
    }

    // Replace the function's type with the new type having updated arguments
    new_func_type->setReturnType(func_type->returnType());
    func_declaration->setAbstractType(new_func_type.cast<AbstractType>());

    return false;   // findType has already explored node
}

/*
 * UI
 */
bool DeclarationBuilder::visit(QmlJS::AST::UiObjectDefinition* node)
{
    setComment(node);

    const DeclarationId id(QualifiedIdentifier(node->qualifiedTypeNameId->name.toString()));
    ///TODO: find type, potentially in C++
    StructureType::Ptr type(new StructureType);
    type->setDeclarationId(id);

    const RangeInRevision range = m_session->locationToRange(node->qualifiedTypeNameId->identifierToken);
    const QualifiedIdentifier& identifier(QmlJS::getQMLAttribute(node->initializer->members, "id"));

    {
        DUChainWriteLocker lock;
        ClassDeclaration* decl = openDeclaration<ClassDeclaration>(identifier, range);
        decl->setKind(Declaration::Type);
    }
    openType(type);

    return DeclarationBuilderBase::visit(node);
}

void DeclarationBuilder::endVisit(QmlJS::AST::UiObjectDefinition* node)
{
    DeclarationBuilderBase::endVisit(node);

    closeAndAssignType();
}

bool DeclarationBuilder::visit(QmlJS::AST::UiObjectInitializer* node)
{
    const bool ret = DeclarationBuilderBase::visit(node);
    DUChainWriteLocker lock;
    if (currentDeclaration<ClassDeclaration>()) {
        Q_ASSERT(currentContext());
        currentDeclaration()->setInternalContext(currentContext());
    }
    return ret;
}

bool DeclarationBuilder::visit(QmlJS::AST::UiScriptBinding* node)
{
    if (node->qualifiedId && node->qualifiedId->name != QLatin1String("id")) {
        setComment(node);

        const RangeInRevision& range = m_session->locationToRange(node->qualifiedId->identifierToken);
        const QualifiedIdentifier id(node->qualifiedId->name.toString());
        const AbstractType::Ptr type(findType(node->statement).type);

        {
            DUChainWriteLocker lock;
            openDeclaration<ClassMemberDeclaration>(id, range);
        }
        openType(type);

        return false;   // findType has already explored node->statement
    }

    return DeclarationBuilderBase::visit(node);
}

void DeclarationBuilder::endVisit(QmlJS::AST::UiScriptBinding* node)
{
    DeclarationBuilderBase::endVisit(node);

    if (node->qualifiedId && node->qualifiedId->name != QLatin1String("id")) {
        closeAndAssignType();
    }
}

bool DeclarationBuilder::visit(QmlJS::AST::UiPublicMember* node)
{
    setComment(node);

    const RangeInRevision& range = m_session->locationToRange(node->identifierToken);
    const QualifiedIdentifier id(node->name.toString());
    const AbstractType::Ptr type = findType(node->statement).type;

    {
        DUChainWriteLocker lock;
        openDeclaration<ClassMemberDeclaration>(id, range);
    }
    openType(type);

    return DeclarationBuilderBase::visit(node);
}

void DeclarationBuilder::endVisit(QmlJS::AST::UiPublicMember* node)
{
    DeclarationBuilderBase::endVisit(node);

    closeAndAssignType();
}

/*
 * Utils
 */
void DeclarationBuilder::closeContext()
{
    {
        DUChainWriteLocker lock;
        FunctionDeclaration* function = dynamic_cast<FunctionDeclaration*>(currentDeclaration());
        DUContext* ctx = currentContext();
        if (function && ctx) {
            if (ctx->type() == DUContext::Function) {
                // This context contains the declarations of the arguments
                function->setInternalFunctionContext(ctx);
            } else {
                // This one contains the body of the function
                Q_ASSERT(ctx->type() == DUContext::Other);
                function->setInternalContext(ctx);
            }
        }
    }
    DeclarationBuilderBase::closeContext();
}

void DeclarationBuilder::setComment(QmlJS::AST::Node* node)
{
    setComment(m_session->commentForLocation(node->firstSourceLocation()).toUtf8());
}

void DeclarationBuilder::closeAndAssignType()
{
    closeType();
    Declaration* dec = currentDeclaration();
    Q_ASSERT(dec);
    Q_ASSERT(lastType());
    {
        DUChainWriteLocker lock;
        dec->setType(lastType());
    }
    closeDeclaration();
}
