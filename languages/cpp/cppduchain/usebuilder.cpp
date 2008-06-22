/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#include "usebuilder.h"

#include <ktexteditor/smartrange.h>
#include <ktexteditor/smartinterface.h>

#include "cppeditorintegrator.h"
#include "name_compiler.h"
#include <declaration.h>
#include <use.h>
#include <topducontext.h>
#include <duchain.h>
#include <duchainlock.h>

#include "expressionvisitor.h"

using namespace KTextEditor;
using namespace KDevelop;

UseBuilder::UseBuilder (ParseSession* session)
  : UseBuilderBase(), m_finishContext(true)
{
  setEditor(new CppEditorIntegrator(session), true);
}

UseBuilder::UseBuilder (CppEditorIntegrator* editor)
  : UseBuilderBase(), m_finishContext(true)
{
  setEditor(editor, false);
}

UseBuilder::~UseBuilder()
{
}

void UseBuilder::buildUses(AST *node)
{
  TopDUContextPointer topContext;
  {
    DUChainReadLocker lock(DUChain::lock());
    topContext = TopDUContextPointer(node->ducontext->topContext());
  }
  //We will have some caching in TopDUContext until this objects lifetime is over
  TopDUContext::Cache cache(topContext);

  UseBuilderBase::buildUses(node);
}

void UseBuilder::visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST * exp) {
  if( exp->expressionChosen )
    visitExpression(exp->expression);
  else
    visit(exp->declaration);
}

void UseBuilder::visitCondition(ConditionAST *node)
{
  visit(node->type_specifier);
  visit(node->declarator);
  visit(node->expression);
}

void UseBuilder::visitExpressionStatement(ExpressionStatementAST * exp) {
  visitExpression(exp);
}

void UseBuilder::visitBinaryExpression(BinaryExpressionAST * exp) {
  visitExpression(exp);
}
void UseBuilder::visitCastExpression(CastExpressionAST * exp) {
  visitExpression(exp);
}
void UseBuilder::visitConditionalExpression(ConditionalExpressionAST * exp) {
  visitExpression(exp);
}
void UseBuilder::visitCppCastExpression(CppCastExpressionAST * exp) {
  visitExpression(exp);
}
void UseBuilder::visitNewExpression(NewExpressionAST * exp) {
  visitExpression(exp);
}
void UseBuilder::visitPostfixExpression(PostfixExpressionAST * exp) {
  visitExpression(exp);
}
void UseBuilder::visitSizeofExpression(SizeofExpressionAST * exp) {
  visitExpression(exp);
}
void UseBuilder::visitSubscriptExpression(SubscriptExpressionAST * exp) {
  visitExpression(exp);
}
void UseBuilder::visitUnaryExpression(UnaryExpressionAST * exp) {
  visitExpression(exp);
}

void UseBuilder::visitPrimaryExpression (PrimaryExpressionAST* exp)
{
  visitExpression(exp);
/*  UseBuilderBase::visitPrimaryExpression(node);

  if (node->name)
    newUse(node->name);*/
}

void UseBuilder::visitMemInitializer(MemInitializerAST * node)
{
  UseBuilderBase::visitMemInitializer(node);

  if (node->initializer_id)
    newUse(node->initializer_id);
}

class UseExpressionVisitor : public Cpp::ExpressionVisitor {
  public:
  UseExpressionVisitor(ParseSession* session, UseBuilder* useBuilder, bool dumpProblems = false) : Cpp::ExpressionVisitor(session), m_builder(useBuilder), m_lastEndToken(0), m_dumpProblems(dumpProblems) {
  }
  private:

  virtual void usingDeclaration( AST* /*node*/, size_t start_token, size_t end_token, const KDevelop::DeclarationPointer& decl ) {
      m_builder->newUse(m_builder->editor()->findRange(start_token, end_token), decl.data());
    }
//     virtual void expressionType( AST* node, const AbstractType::Ptr& type, Instance instance )
//     {
//       std::size_t start = node->start_token, end = node->end_token;
// 
//       if(start < m_lastEndToken)
//         start = m_lastEndToken;
// 
//       if(start > end) {
//         //We are expecting the processing from left to right atm, which is not right. We should be more precise.
//         problem(node, QString("Use-range overlap, m_lastEndToken: %1 node-start: %2 node-end: %3" ).arg(m_lastEndToken).arg(node->start_token).arg(node->end_token));
//         return;
//       }
// 
//       Declaration* decl = instance.declaration;
//       if(!decl && dynamic_cast<const IdentifiedType*>(type.data()))
//         decl = dynamic_cast<const IdentifiedType*>(type.data())->declaration();
// 
//       if(!instance.declaration && type && !dynamic_cast<const IdentifiedType*>(type.data())) {
//         //Non-identified types do not have declarations, and can be integer-, string-literals, etc.
//       } else {
//         m_builder->newUse(start, end, decl);
//       }
//     }

    virtual void problem(AST* node, const QString& str) {
      if(m_dumpProblems)
        Cpp::ExpressionVisitor::problem(node, str);
/*      else
        kDebug(9007) << "problem";*/
    }

    UseBuilder* m_builder;
    std::size_t m_lastEndToken; //Assume everything is processed from left to right
    bool m_dumpProblems;
};

void UseBuilder::visitExpression(AST* node) {
  UseExpressionVisitor visitor( editor()->parseSession(), this );
  if( !node->ducontext )
    node->ducontext = currentContext();
  
  visitor.parse( node );
}

void UseBuilder::visitBaseSpecifier(BaseSpecifierAST* node) {
  UseExpressionVisitor visitor( editor()->parseSession(), this );
  if(node->name) {
    if( !node->name->ducontext )
      node->name->ducontext = currentContext();
    
    visitor.parse( node->name );
  }
}

void UseBuilder::visitSimpleTypeSpecifier(SimpleTypeSpecifierAST* node)
{
  UseBuilderBase::visitSimpleTypeSpecifier(node);
  
  UseExpressionVisitor visitor( editor()->parseSession(), this );
  if( !node->ducontext )
    node->ducontext = currentContext();
  
  visitor.parse( node );
}

void UseBuilder::visitDeclarator(DeclaratorAST* node)
{
  if(node->id) {
    UseExpressionVisitor visitor( editor()->parseSession(), this );
    if( !node->id->ducontext )
      node->id->ducontext = currentContext();

    visitor.parseNamePrefix(node->id);
  }

  UseBuilderBase::visitDeclarator(node);
}

void UseBuilder::visitClassSpecifier(ClassSpecifierAST* node)
{
  if(node->name) {
    UseExpressionVisitor visitor( editor()->parseSession(), this );
    if( !node->name->ducontext )
      node->name->ducontext = currentContext();

    visitor.parseNamePrefix(node->name);
  }

  UseBuilderBase::visitClassSpecifier(node);
}

void UseBuilder::visitTypeId(TypeIdAST* node) {
  {
    UseExpressionVisitor visitor( editor()->parseSession(), this );
    if( !node->ducontext )
      node->ducontext = currentContext();

    visitor.parse( node );
  }
  
  UseBuilderBase::visitTypeId(node);
}

void UseBuilder::visitUsing(UsingAST *node)
{
  if(node->name) {
    UseExpressionVisitor visitor( editor()->parseSession(), this );
    if( !node->name->ducontext )
      node->name->ducontext = currentContext();

    visitor.parse( node->name );
  }
  UseBuilderBase::visitUsing(node);
}
