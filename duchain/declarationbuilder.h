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

#ifndef DECLARATIONBUILDER_H
#define DECLARATIONBUILDER_H

#include <language/duchain/builders/abstractdeclarationbuilder.h>
#include <language/duchain/builders/abstracttypebuilder.h>

#include "contextbuilder.h"

typedef KDevelop::AbstractTypeBuilder<QmlJS::AST::Node, QmlJS::AST::IdentifierPropertyName, ContextBuilder> TypeBuilder;
typedef KDevelop::AbstractDeclarationBuilder<QmlJS::AST::Node, QmlJS::AST::IdentifierPropertyName, TypeBuilder> DeclarationBuilderBase;

class KDEVQMLJSDUCHAIN_EXPORT DeclarationBuilder : public DeclarationBuilderBase
{
public:
    DeclarationBuilder(ParseSession* session);

    virtual KDevelop::ReferencedTopDUContext build(const KDevelop::IndexedString& url,
                                                   QmlJS::AST::Node* node,
                                                   KDevelop::ReferencedTopDUContext updateContext = KDevelop::ReferencedTopDUContext());

protected:
    using Visitor::visit;
    using Visitor::endVisit;

    // Functions
    void visitFunction(QmlJS::AST::FunctionExpression* node);
    void endVisitFunction(QmlJS::AST::FunctionExpression*);

    virtual bool visit(QmlJS::AST::FunctionDeclaration* node);
    virtual bool visit(QmlJS::AST::FunctionExpression* node);
    virtual bool visit(QmlJS::AST::FormalParameterList* node);
    virtual bool visit(QmlJS::AST::ReturnStatement* node);
    virtual void endVisit(QmlJS::AST::FunctionDeclaration* node);
    virtual void endVisit(QmlJS::AST::FunctionExpression* node);

    // Variables
    virtual bool visit(QmlJS::AST::VariableDeclaration* node);
    virtual void endVisit(QmlJS::AST::VariableDeclaration* node);
    virtual bool visit(QmlJS::AST::BinaryExpression* node);
    virtual bool visit(QmlJS::AST::CallExpression* node);

    // UI
    virtual bool visit(QmlJS::AST::UiObjectDefinition* node);
    virtual void endVisit(QmlJS::AST::UiObjectDefinition* node);

    virtual bool visit(QmlJS::AST::UiObjectInitializer* node);

    virtual bool visit(QmlJS::AST::UiScriptBinding* node);
    virtual void endVisit(QmlJS::AST::UiScriptBinding* node);

    virtual bool visit(QmlJS::AST::UiPublicMember* node);
    virtual void endVisit(QmlJS::AST::UiPublicMember* node);

    virtual void closeContext();

private:
    void closeAndAssignType();
    KDevelop::AbstractType::Ptr typeFromName(const QString& name);          /*!< @brief Type from a general name (int, string, or a class name) */
    KDevelop::AbstractType::Ptr typeFromClassName(const QString& name);     /*!< @brief Type from a class name, built-in types are not supported here */

    using DeclarationBuilderBase::setComment;
    void setComment(QmlJS::AST::Node* node);
};

#endif // DECLARATIONBUILDER_H
