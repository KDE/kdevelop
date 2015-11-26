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
#include <language/checks/dataaccessrepository.h>
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
    virtual void visitUnqualifiedName(UnqualifiedNameAST* node) override;
    virtual void visitFunctionCall(FunctionCallAST* node) override;
    virtual void visitBinaryExpression(BinaryExpressionAST* node) override;
    
//     virtual void visitCastExpression(CastExpressionAST *) ;
    virtual void visitClassMemberAccess(ClassMemberAccessAST *) override ;
    virtual void visitCondition(ConditionAST *) override ;
    virtual void visitConditionalExpression(ConditionalExpressionAST *) override ;
//     virtual void visitCppCastExpression(CppCastExpressionAST *) ;
    virtual void visitDeleteExpression(DeleteExpressionAST *) override ;
    virtual void visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST *) override ;
    virtual void visitIncrDecrExpression(IncrDecrExpressionAST *) override ;
    virtual void visitInitDeclarator(InitDeclaratorAST *) override ;
    virtual void visitMemInitializer(MemInitializerAST *) override ;
    virtual void visitNewExpression(NewExpressionAST *) override ;
    virtual void visitPostfixExpression(PostfixExpressionAST *) override ;
    virtual void visitReturnStatement(ReturnStatementAST* ) override ;
//     virtual void visitSimpleDeclaration(SimpleDeclarationAST *) ;
//     virtual void visitThrowExpression(ThrowExpressionAST *) ;
    virtual void visitUnaryExpression(UnaryExpressionAST *) override ;
    virtual void visitCppCastExpression(CppCastExpressionAST* ) override;
    virtual void visitInitializerList(InitializerListAST* ) override;
    
  private:
    KDevelop::CursorInRevision cursorForToken(uint token);
    KDevelop::RangeInRevision rangeForNode(AST* ast);
    QString nodeToString(AST* node);
    
    const ParseSession* m_session;
    QStack< QList<KDevelop::DataAccess::DataAccessFlags> > m_callStack;
    QStack<int> m_argStack;
    KDevelop::DataAccess::DataAccessFlags m_defaultFlags;
    KDevelop::DataAccessRepository* m_mods;
};

#endif // USEDECORATORVISITOR_H
