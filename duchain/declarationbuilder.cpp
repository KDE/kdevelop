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
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/classdeclaration.h>

#include "parsesession.h"

using namespace KDevelop;

namespace {

/**
 * Find the 'id: <identifier>' in a given object member list.
 *
 * This is a bit ugly, and a Visitor is seemlingly more appropriate
 * for the task. But it is very hard to write it (lots of code) such
 * that it does not recurse into any other node...
 */
QualifiedIdentifier findIdentifier(QmlJS::AST::UiObjectMemberList* members)
{
    for (QmlJS::AST::UiObjectMemberList *it = members; it; it = it->next) {
        QmlJS::AST::UiScriptBinding* binding = QmlJS::AST::cast<QmlJS::AST::UiScriptBinding*>(it->member);
        if (!binding || !binding->qualifiedId || binding->qualifiedId->name != QLatin1String("id")) {
            continue;
        }
        QmlJS::AST::ExpressionStatement* statement = QmlJS::AST::cast<QmlJS::AST::ExpressionStatement*>(binding->statement);
        if (!statement) {
            continue;
        }
        QmlJS::AST::IdentifierExpression* identifier = QmlJS::AST::cast<QmlJS::AST::IdentifierExpression*>(statement->expression);
        if (!identifier) {
            continue;
        }
        return QualifiedIdentifier(identifier->name.toString());
    }
    return QualifiedIdentifier();
}

}

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

bool DeclarationBuilder::visit(QmlJS::AST::FunctionDeclaration* node)
{
    setComment(m_session->commentForLocation(node->firstSourceLocation()).toUtf8());

    const QualifiedIdentifier name(node->name.toString());
    const RangeInRevision range = m_session->locationToRange(node->identifierToken);

    FunctionType::Ptr type(new FunctionType);
    type->setReturnType(AbstractType::Ptr(new IntegralType(IntegralType::TypeVoid)));

    {
        DUChainWriteLocker lock;
        FunctionDeclaration* fun = openDeclaration<FunctionDeclaration>(name, range);
        fun->setType(type);
        openType(type);
    }

    return DeclarationBuilderBase::visit(node);
}

void DeclarationBuilder::endVisit(QmlJS::AST::FunctionDeclaration* node)
{
    DeclarationBuilderBase::endVisit(node);

    closeType();
    DUChainWriteLocker lock;
    Declaration* last = currentDeclaration();
    closeDeclaration();
    last->setType(lastType());
}

bool DeclarationBuilder::visit(QmlJS::AST::FormalParameterList* node)
{
    for (QmlJS::AST::FormalParameterList *plist = node; plist; plist = plist->next) {
        const QualifiedIdentifier name(plist->name.toString());
        const RangeInRevision range = m_session->locationToRange(plist->identifierToken);
        DUChainWriteLocker lock;
        Declaration* dec = openDeclaration<Declaration>(name, range);
        IntegralType* type = new IntegralType(IntegralType::TypeMixed);
        dec->setType(IntegralType::Ptr(type));
        closeDeclaration();
    }

    return DeclarationBuilderBase::visit(node);
}

bool DeclarationBuilder::visit(QmlJS::AST::VariableDeclaration* node)
{
    setComment(m_session->commentForLocation(node->firstSourceLocation()).toUtf8());

    const QualifiedIdentifier name(node->name.toString());
    const RangeInRevision range = m_session->locationToRange(node->identifierToken);
    DUChainWriteLocker lock;
    Declaration* dec = openDeclaration<Declaration>(name, range);
    IntegralType* type = new IntegralType(IntegralType::TypeMixed);
    dec->setType(IntegralType::Ptr(type));
    closeDeclaration();

    return DeclarationBuilderBase::visit(node);
}

void DeclarationBuilder::closeContext()
{
    DUChainWriteLocker lock;
    if (FunctionDeclaration* function = dynamic_cast<FunctionDeclaration*>(currentDeclaration())) {
        DUContext* ctx = currentContext();
        if (ctx->type() == DUContext::Function) {
            function->setInternalFunctionContext(ctx);
        } else {
            Q_ASSERT(ctx->type() == DUContext::Other);
            function->setInternalContext(ctx);
        }
    }
    DeclarationBuilderBase::closeContext();
}

bool DeclarationBuilder::visit(QmlJS::AST::UiObjectDefinition* node)
{
    setComment(node);

    ///TODO: find type, potentially in C++
    StructureType::Ptr type(new StructureType);
    DeclarationId id(QualifiedIdentifier(node->qualifiedTypeNameId->name.toString()));
    type->setDeclarationId(id);

    const RangeInRevision range = m_session->locationToRange(node->qualifiedTypeNameId->identifierToken);
    const QualifiedIdentifier& identifier = findIdentifier(node->initializer->members);

    {
        DUChainWriteLocker lock;
        ClassDeclaration* decl = openDeclaration<ClassDeclaration>(identifier, range);
        decl->setKind(Declaration::Type);
        decl->setType(type);
        openType(type);
    }

    return DeclarationBuilderBase::visit(node);
}

void DeclarationBuilder::endVisit(QmlJS::AST::UiObjectDefinition* node)
{
    DeclarationBuilderBase::endVisit(node);

    closeType();
    closeDeclaration();
}

bool DeclarationBuilder::visit(QmlJS::AST::UiObjectInitializer* node)
{
    bool ret = DeclarationBuilderBase::visit(node);
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
        const AbstractType::Ptr type(new IntegralType(IntegralType::TypeMixed));

        DUChainWriteLocker lock;
        {
            ClassMemberDeclaration* dec = openDeclaration<ClassMemberDeclaration>(id, range);
            dec->setAbstractType(type);
        }
        openType(type);
    }
    return DeclarationBuilderBase::visit(node);
}

void DeclarationBuilder::endVisit(QmlJS::AST::UiScriptBinding* node)
{
    DeclarationBuilderBase::endVisit(node);

    if (node->qualifiedId && node->qualifiedId->name != QLatin1String("id")) {
        closeType();
        closeDeclaration();
    }
}

bool DeclarationBuilder::visit(QmlJS::AST::UiPublicMember* node)
{
    setComment(node);

    const RangeInRevision& range = m_session->locationToRange(node->identifierToken);
    const QualifiedIdentifier id(node->name.toString());
    const AbstractType::Ptr type(new IntegralType(IntegralType::TypeMixed));

    {
        DUChainWriteLocker lock;
        ClassMemberDeclaration* dec = openDeclaration<ClassMemberDeclaration>(id, range);
        dec->setAbstractType(type);
    }
    openType(type);

    return DeclarationBuilderBase::visit(node);
}

void DeclarationBuilder::endVisit(QmlJS::AST::UiPublicMember* /*node*/)
{
    closeType();
    closeDeclaration();
}

void DeclarationBuilder::setComment(QmlJS::AST::Node* node)
{
    setComment(m_session->commentForLocation(node->firstSourceLocation()).toUtf8());
}
