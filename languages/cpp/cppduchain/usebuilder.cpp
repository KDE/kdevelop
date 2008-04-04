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
  : UseBuilderBase(session), m_finishContext(true)
{
}

UseBuilder::UseBuilder (CppEditorIntegrator* editor)
  : UseBuilderBase(editor), m_finishContext(true)
{
}

void UseBuilder::buildUses(AST *node)
{
  TopDUContext* top = dynamic_cast<TopDUContext*>(node->ducontext);

  if (top) {
    DUChainWriteLocker lock(DUChain::lock());
    top->clearDeclarationIndices();
    if(top->hasUses())
      setRecompiling(true);
  }
  
  supportBuild(node);

  if (top)
    top->setHasUses(true);
}

void UseBuilder::newUse(NameAST* name)
{
  QualifiedIdentifier id = identifierForName(name);

  SimpleRange newRange = m_editor->findRange(name);
  
  DUChainWriteLocker lock(DUChain::lock());
  QList<Declaration*> declarations = currentContext()->findDeclarations(id, newRange.start);
  foreach (Declaration* declaration, declarations)
    if (!declaration->isForwardDeclaration()) {
      declarations.clear();
      declarations.append(declaration);
      break;
    }
  // If we don't break, there's no non-forward declaration

  lock.unlock();
  newUse( name->start_token, name->end_token, !declarations.isEmpty() ? declarations.first() : 0 );
}

void UseBuilder::newUse(std::size_t start_token, std::size_t end_token, KDevelop::Declaration* declaration)
{
  DUChainWriteLocker lock(DUChain::lock());

  if(!declaration) {
    kDebug(9007) << "Tried to create use of zero declaration";
    return;
  }
  
  SimpleRange newRange = m_editor->findRange(start_token, end_token);

  /**
   * We need to find a context that this use fits into, which must not necessarily be the current one.
   * The reason are macros like SOME_MACRO(SomeClass), where SomeClass is expanded to be within a
   * sub-context that comes from the macro. That sub-context will have a very small range, and will most
   * probably not be the range of the actual "SomeClass" text, so the "SomeClass" use has to be moved
   * into the context that surrounds the SOME_MACRO invocation.
   * */
  DUContext* newContext = currentContext();
  int contextUpSteps = 0;
  while (!newContext->range().contains(newRange) && newContext->parentContext()) {
    newContext = newContext->parentContext();
    ++contextUpSteps;
  }

  if (contextUpSteps) {
    openContext(newContext);
    nextUseIndex() = m_nextUseStack.at(m_nextUseStack.size()-contextUpSteps-2);
    skippedUses() = m_skippedUses.at(m_skippedUses.size()-contextUpSteps-2);
    
    Q_ASSERT(m_contexts[m_nextUseStack.size()-contextUpSteps-2] == currentContext());
    Q_ASSERT(currentContext()->uses().count() >= nextUseIndex());
  }
  
  bool encountered = false;

  int declarationIndex = currentContext()->topContext()->indexForUsedDeclaration(declaration);

  if (recompiling()) {

    const QVector<Use>& uses = currentContext()->uses();
    // Translate cursor to take into account any changes the user may have made since the text was retrieved
    SimpleRange translated = newRange;
    if (m_editor->smart()) {
      lock.unlock();
      QMutexLocker smartLock(m_editor->smart()->smartMutex());
      translated = SimpleRange( m_editor->smart()->translateFromRevision(translated.textRange()) );
      lock.lock();
    }

    for (; nextUseIndex() < uses.count(); ++nextUseIndex()) {
      const Use& use = uses[nextUseIndex()];

      if (use.m_range.start > translated.end && m_editor->smart() )
        break;

      if (use.m_range == translated)
      {
        currentContext()->setUseDeclaration(nextUseIndex(), declarationIndex);
        ++nextUseIndex();
        // Match
        encountered = true;

        break;
      }
      //Not encountered, and before the current range. Remove all intermediate uses.
      skippedUses().append(nextUseIndex());
    }
  }

  if (!encountered) {

    SmartRange* prior = m_editor->currentRange();

    //We must close all ranges in the range-hierarchy that belong to context that we have moved this use out of
    QList<SmartRange*> backupRanges;
    for(int a = 0; a < contextUpSteps; a++) {
      SmartRange* s = m_editor->currentRange();
      if(s)
        backupRanges.push_front(s);
      m_editor->exitCurrentRange();
    }

    SmartRange* use = m_editor->createRange(newRange.textRange());
    m_editor->exitCurrentRange();

    currentContext()->createUse(declarationIndex, newRange, use, nextUseIndex());
    ++nextUseIndex();

    for (QList<SmartRange*>::const_iterator it = backupRanges.begin(); it != backupRanges.end(); ++it)
      m_editor->setCurrentRange( *it );

    Q_ASSERT(m_editor->currentRange() == prior);
  }

  if (contextUpSteps) {
    Q_ASSERT(m_contexts[m_nextUseStack.size()-contextUpSteps-2] == currentContext());
    Q_ASSERT(currentContext()->uses().count() >= nextUseIndex());
    m_nextUseStack[m_nextUseStack.size()-contextUpSteps-2] = nextUseIndex();
    m_skippedUses[m_skippedUses.size()-contextUpSteps-2] = skippedUses();
    m_finishContext = false;
    closeContext();
    m_finishContext = true;
  }
}

void UseBuilder::openContext(DUContext * newContext)
{
  UseBuilderBase::openContext(newContext);

  m_contexts.push(newContext);
  m_nextUseStack.push(0);
  m_skippedUses.push(QVector<int>());
}

void UseBuilder::closeContext()
{
  if(m_finishContext) {
    DUChainWriteLocker lock(DUChain::lock());
    
    //Delete all uses that were not encountered
    //That means: All uses in skippedUses, and all uses from nextUseIndex() to currentContext()->uses().count()
    for(int a = currentContext()->uses().count()-1; a >= nextUseIndex(); --a)
      currentContext()->deleteUse(a);
    for(int a = skippedUses().count()-1; a >= 0; --a)
      currentContext()->deleteUse(skippedUses()[a]);
  }
  
  UseBuilderBase::closeContext();

  m_contexts.pop();
  m_nextUseStack.pop();
  m_skippedUses.pop();
}

void UseBuilder::visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST * exp) {
  if( exp->expressionChosen )
    visitExpression(exp->expression);
  else
    visit(exp->declaration);
}

void UseBuilder::visitCondition(ConditionAST *node)
{
  ///@todo Until we find out how to correctly handle this type-specifier(It is created wrongly in test_duchain.cpp testDeclareFor) ignore it.
  //visit(node->type_specifier);
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
  UseExpressionVisitor(ParseSession* session, UseBuilder* useBuilder, bool dumpProblems = false) : Cpp::ExpressionVisitor(session, ImportTrace()), m_builder(useBuilder), m_lastEndToken(0), m_dumpProblems(dumpProblems) {
  }
  private:

  virtual void usingDeclaration( AST* /*node*/, size_t start_token, size_t end_token, const KDevelop::DeclarationPointer& decl ) {
      m_builder->newUse(start_token, end_token, decl.data());
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
  UseExpressionVisitor visitor( m_editor->parseSession(), this );
  if( !node->ducontext )
    node->ducontext = currentContext();
  
  visitor.parse( node );
}

void UseBuilder::visitBaseSpecifier(BaseSpecifierAST* node) {
  UseExpressionVisitor visitor( m_editor->parseSession(), this );
  if(node->name) {
    if( !node->name->ducontext )
      node->name->ducontext = currentContext();
    
    visitor.parse( node->name );
  }
}

void UseBuilder::visitSimpleTypeSpecifier(SimpleTypeSpecifierAST* node)
{
  UseBuilderBase::visitSimpleTypeSpecifier(node);
  
  UseExpressionVisitor visitor( m_editor->parseSession(), this );
  if( !node->ducontext )
    node->ducontext = currentContext();
  
  visitor.parse( node );
}

void UseBuilder::visitDeclarator(DeclaratorAST* node)
{
  if(node->id) {
    UseExpressionVisitor visitor( m_editor->parseSession(), this );
    if( !node->id->ducontext )
      node->id->ducontext = currentContext();

    visitor.parseNamePrefix(node->id);
  }

  UseBuilderBase::visitDeclarator(node);
}

void UseBuilder::visitClassSpecifier(ClassSpecifierAST* node)
{
  if(node->name) {
    UseExpressionVisitor visitor( m_editor->parseSession(), this );
    if( !node->name->ducontext )
      node->name->ducontext = currentContext();

    visitor.parseNamePrefix(node->name);
  }

  UseBuilderBase::visitClassSpecifier(node);
}
