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

#include <util/stack.h>

namespace KDevelop
{
    class ClassDeclaration;
}

namespace QmlJS
{
    class NodeJS;
}

typedef KDevelop::AbstractTypeBuilder<QmlJS::AST::Node, QmlJS::AST::IdentifierPropertyName, ContextBuilder> TypeBuilder;
typedef KDevelop::AbstractDeclarationBuilder<QmlJS::AST::Node, QmlJS::AST::IdentifierPropertyName, TypeBuilder> DeclarationBuilderBase;

class KDEVQMLJSDUCHAIN_EXPORT DeclarationBuilder : public DeclarationBuilderBase
{
    friend class QmlJS::NodeJS;

public:
    explicit DeclarationBuilder(ParseSession* session);

    KDevelop::ReferencedTopDUContext build(const KDevelop::IndexedString& url,
                                           QmlJS::AST::Node* node,
                                           KDevelop::ReferencedTopDUContext updateContext = KDevelop::ReferencedTopDUContext()) override;
    void startVisiting(QmlJS::AST::Node* node) override;

protected:
    using Visitor::visit;
    using Visitor::endVisit;

    typedef QList<QPair<QmlJS::AST::StringLiteral*, QString>> ExportLiteralsAndNames;

    // Functions
    template<typename Decl>
    void declareFunction(QmlJS::AST::Node* node,
                         bool newPrototypeContext,
                         const KDevelop::Identifier& name,
                         const KDevelop::RangeInRevision& nameRange,
                         QmlJS::AST::Node* parameters,
                         const KDevelop::RangeInRevision& parametersRange,
                         QmlJS::AST::Node* body,
                         const KDevelop::RangeInRevision& bodyRange);
    template<typename Node>
    void declareParameters(Node* node, QStringRef Node::*typeAttribute);
    void endVisitFunction();    // Set the return type of the function to void if no return statement has been encountered

    bool visit(QmlJS::AST::FunctionDeclaration* node) override;
    bool visit(QmlJS::AST::FunctionExpression* node) override;
    bool visit(QmlJS::AST::FormalParameterList* node) override;
    bool visit(QmlJS::AST::UiParameterList* node) override;
    bool visit(QmlJS::AST::ReturnStatement* node) override;
    void endVisit(QmlJS::AST::FunctionDeclaration* node) override;
    void endVisit(QmlJS::AST::FunctionExpression* node) override;

    // Variables
    /// NOTE: this visits the @p base node and its children
    void inferArgumentsFromCall(QmlJS::AST::Node* base,
                                QmlJS::AST::ArgumentList* arguments);
    bool visit(QmlJS::AST::VariableDeclaration* node) override;
    void endVisit(QmlJS::AST::VariableDeclaration* node) override;
    bool visit(QmlJS::AST::BinaryExpression* node) override;
    bool visit(QmlJS::AST::CallExpression* node) override;
    bool visit(QmlJS::AST::NewMemberExpression* node) override;

    // Arrays
    void declareFieldMember(const KDevelop::DeclarationPointer& declaration,
                            const QString& member,
                            QmlJS::AST::Node* node,
                            const QmlJS::AST::SourceLocation& location);
    bool visit(QmlJS::AST::FieldMemberExpression* node) override;
    bool visit(QmlJS::AST::ArrayMemberExpression* node) override;

    bool visit(QmlJS::AST::ObjectLiteral* node) override;
    bool visit(QmlJS::AST::PropertyNameAndValue* node) override;
    void endVisit(QmlJS::AST::PropertyNameAndValue* node) override;
    void endVisit(QmlJS::AST::ObjectLiteral* node) override;

    // plugin.qmltypes
    void declareComponent(QmlJS::AST::UiObjectInitializer* node,
                          const KDevelop::RangeInRevision &range,
                          const KDevelop::Identifier &name);
    void declareMethod(QmlJS::AST::UiObjectInitializer* node,
                       const KDevelop::RangeInRevision &range,
                       const KDevelop::Identifier &name,
                       bool isSlot,
                       bool isSignal);
    void declareProperty(QmlJS::AST::UiObjectInitializer* node,
                         const KDevelop::RangeInRevision &range,
                         const KDevelop::Identifier &name);
    void declareParameter(QmlJS::AST::UiObjectInitializer* node,
                          const KDevelop::RangeInRevision &range,
                          const KDevelop::Identifier &name);
    void declareEnum(const KDevelop::RangeInRevision &range,
                     const KDevelop::Identifier &name);
    void declareComponentSubclass(QmlJS::AST::UiObjectInitializer* node,
                                  const KDevelop::RangeInRevision& range,
                                  const QString& baseclass,
                                  QmlJS::AST::UiQualifiedId* qualifiedId);
    void declareComponentInstance(QmlJS::AST::ExpressionStatement *expression);
    ExportLiteralsAndNames exportedNames(QmlJS::AST::ExpressionStatement *exports);
    void declareExports(const ExportLiteralsAndNames& exports,
                        KDevelop::ClassDeclaration* classdecl);

    // UI
    void importDirectory(const QString& directory, QmlJS::AST::UiImport* node);
    void importModule(QmlJS::AST::UiImport* node);
    bool visit(QmlJS::AST::UiImport* node) override;

    bool visit(QmlJS::AST::UiObjectDefinition* node) override;
    void endVisit(QmlJS::AST::UiObjectDefinition* node) override;

    bool visit(QmlJS::AST::UiScriptBinding* node) override;
    void endVisit(QmlJS::AST::UiScriptBinding* node) override;
    bool visit(QmlJS::AST::UiObjectBinding* node) override;
    void endVisit(QmlJS::AST::UiObjectBinding* node) override;

    bool visit(QmlJS::AST::UiPublicMember* node) override;
    void endVisit(QmlJS::AST::UiPublicMember* node) override;

protected:
    template<class DeclarationT>
    DeclarationT* openDeclaration(const KDevelop::Identifier& id,
                                  const KDevelop::RangeInRevision& newRange,
                                  DeclarationFlags flags = NoFlags)
    {
        DeclarationT* res = DeclarationBuilderBase::openDeclaration<DeclarationT>(id, newRange, flags);
        res->setAlwaysForceDirect(true);
        return res;
    }

private:
    void closeAndAssignType();
    void registerBaseClasses();              /*!< @brief Enumerates the base classes of the current class and import their inner contexts */
    void addBaseClass(KDevelop::ClassDeclaration* classDecl, const QString &name);    /*!< @brief Add a base class to a class declaration */
    void addBaseClass(KDevelop::ClassDeclaration* classDecl, const KDevelop::IndexedType& type);
    KDevelop::AbstractType::Ptr typeFromName(const QString& name);          /*!< @brief Type from a general name (int, string, or a class name) */
    KDevelop::AbstractType::Ptr typeFromClassName(const QString& name);     /*!< @brief Type from a class name, built-in types are not supported here */
    bool areTypesEqual(const KDevelop::AbstractType::Ptr& a,
                       const KDevelop::AbstractType::Ptr& b);

    using DeclarationBuilderBase::setComment;
    void setComment(QmlJS::AST::Node* node);

private:
    bool m_prebuilding;
    KDevelop::Stack<bool> m_skipEndVisit;
};

#endif // DECLARATIONBUILDER_H
