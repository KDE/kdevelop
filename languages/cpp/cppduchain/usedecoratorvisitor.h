/* This file is part of KDevelop
    Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef USEDECORATORVISITOR_H
#define USEDECORATORVISITOR_H

#include <cppduchainexport.h>
#include <default_visitor.h>
#include <language/duchain/types/abstracttype.h>
#include <QStack>

namespace KDevelop {
class DataAccessRepository;
}

class KDEVCPPDUCHAIN_EXPORT UseDecoratorVisitor : protected DefaultVisitor
{
  public:
    UseDecoratorVisitor(const ParseSession* session, KDevelop::DataAccessRepository* repo);
    
    void run(AST* node);
  protected:
    virtual void visitUnqualifiedName(UnqualifiedNameAST* node);
    virtual void visitFunctionCall(FunctionCallAST* node);
    virtual void visitBinaryExpression(BinaryExpressionAST* node);
    
    virtual void visitCastExpression(CastExpressionAST *) ;
    virtual void visitClassMemberAccess(ClassMemberAccessAST *) ;
    virtual void visitCompoundStatement(CompoundStatementAST *) ;
    virtual void visitCondition(ConditionAST *) ;
    virtual void visitConditionalExpression(ConditionalExpressionAST *) ;
    virtual void visitCppCastExpression(CppCastExpressionAST *) ;
    virtual void visitDeclarationStatement(DeclarationStatementAST *) ;
    virtual void visitDeclarator(DeclaratorAST *) ;
    virtual void visitDeleteExpression(DeleteExpressionAST *) ;
    virtual void visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST *) ;
    virtual void visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST *) ;
    virtual void visitExpressionStatement(ExpressionStatementAST *) ;
    virtual void visitIncrDecrExpression(IncrDecrExpressionAST *) ;
    virtual void visitInitDeclarator(InitDeclaratorAST *) ;
    virtual void visitMemInitializer(MemInitializerAST *) ;
    virtual void visitName(NameAST *) ;
    virtual void visitNewDeclarator(NewDeclaratorAST *) ;
    virtual void visitNewExpression(NewExpressionAST *) ;
    virtual void visitNewInitializer(NewInitializerAST *) ;
    virtual void visitPostfixExpression(PostfixExpressionAST *) ;
    virtual void visitPrimaryExpression(PrimaryExpressionAST *) ;
    virtual void visitPtrOperator(PtrOperatorAST *) ;
    virtual void visitReturnStatement(ReturnStatementAST* ) ;
    virtual void visitSimpleDeclaration(SimpleDeclarationAST *) ;
    virtual void visitSimpleTypeSpecifier(SimpleTypeSpecifierAST *) ;
    virtual void visitSizeofExpression(SizeofExpressionAST *) ;
    virtual void visitStringLiteral(StringLiteralAST *) ;
    virtual void visitSubscriptExpression(SubscriptExpressionAST *) ;
    virtual void visitThrowExpression(ThrowExpressionAST *) ;
    virtual void visitTranslationUnit(TranslationUnitAST *) ;
    virtual void visitTypeId(TypeIdAST *) ;
    virtual void visitUnaryExpression(UnaryExpressionAST *) ;
    virtual void visitSignalSlotExpression (SignalSlotExpressionAST*);
    
  private:
    const ParseSession* m_session;
    QStack< QList<KDevelop::AbstractType::Ptr> > m_callStack;
    QStack<int> m_argStack;
    KDevelop::DUContext* m_currentContext;
    uint m_defaultFlags;
    KDevelop::DataAccessRepository* m_mods;
};

#endif // USEDECORATORVISITOR_H
