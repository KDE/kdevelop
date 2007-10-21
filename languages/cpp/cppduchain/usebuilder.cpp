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
  : UseBuilderBase(session)
{
}

UseBuilder::UseBuilder (CppEditorIntegrator* editor)
  : UseBuilderBase(editor)
{
}

void UseBuilder::buildUses(AST *node)
{
  supportBuild(node);

  if (TopDUContext* top = dynamic_cast<TopDUContext*>(node->ducontext))
    top->setHasUses(true);
}

void UseBuilder::visitPrimaryExpression (PrimaryExpressionAST* node)
{
  UseBuilderBase::visitPrimaryExpression(node);

  if (node->name)
    newUse(node->name);
}

void UseBuilder::visitMemInitializer(MemInitializerAST * node)
{
  UseBuilderBase::visitMemInitializer(node);

  if (node->initializer_id)
    newUse(node->initializer_id);
}

void UseBuilder::newUse(NameAST* name)
{
  QualifiedIdentifier id = identifierForName(name);

  Range newRange = m_editor->findRange(name);
  
  DUChainWriteLocker lock(DUChain::lock());
  QList<Declaration*> declarations = currentContext()->findDeclarations(id, newRange.start());
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
  
  Range newRange = m_editor->findRange(start_token, end_token);
  
  Use* ret = 0;

  if (recompiling()) {
    const QList<Use*>& uses = currentContext()->uses();

    QMutexLocker smartLock(m_editor->smart() ? m_editor->smart()->smartMutex() : 0);
    // Translate cursor to take into account any changes the user may have made since the text was retrieved
    Range translated = newRange;
    if (m_editor->smart())
      translated = m_editor->smart()->translateFromRevision(translated);

    for (; nextUseIndex() < uses.count(); ++nextUseIndex()) {
      Use* use = uses.at(nextUseIndex());

      if (use->textRange().start() > translated.end() && use->smartRange() )
        break;

      if (use->textRange() == translated &&
          ((!use->declaration() && !declaration) ||
           (declaration && use->declaration() == declaration)))
      {
        // Match
        ret = use;

        setEncountered(ret);
        //Eventually upgrade the range to a smart-range
        /*if( m_editor->smart() && !ret->smartRange() )
          ret->setTextRange(m_editor->createRange(newRange));*/

        break;
      }
    }
  }

  if (!ret) {
    Range* prior = m_editor->currentRange();
    Range* use = m_editor->createRange(newRange);
    m_editor->exitCurrentRange();
    Q_ASSERT(m_editor->currentRange() == prior);

    Use* newUse = new Use(use, currentContext());

    setEncountered(newUse);

    if (declaration)
      declaration->addUse(newUse);
    else
      currentContext()->addOrphanUse(newUse);
      //kWarning(9007) << "Could not find definition for identifier" << id << "at" << *use ;
  }
}

void UseBuilder::openContext(DUContext * newContext)
{
  UseBuilderBase::openContext(newContext);

  m_nextUseStack.push(0);
}

void UseBuilder::closeContext()
{
  UseBuilderBase::closeContext();

  m_nextUseStack.pop();
}

void UseBuilder::visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST * exp) {
  if( exp->expressionChosen )
    visitExpression(exp->expression);
  else
    UseBuilderBase::visitExpressionOrDeclarationStatement(exp);
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


class UseExpressionVisitor : public Cpp::ExpressionVisitor {
  public:
  UseExpressionVisitor(ParseSession* session, UseBuilder* useBuilder, bool dumpProblems = false) : Cpp::ExpressionVisitor(session), m_builder(useBuilder), m_lastEndToken(0), m_dumpProblems(dumpProblems) {
  }
  private:

    virtual void expressionType( AST* node, const AbstractType::Ptr& type, Instance instance )
    {
      std::size_t start = node->start_token, end = node->end_token;

      if(start < m_lastEndToken)
        start = m_lastEndToken;

      if(start > end) {
        //We are expecting the processing from left to right atm, which is not right. We should be more precise.
        problem(node, QString("Use-range overlap, m_lastEndToken: %1 node-start: %2 node-end: %3" ).arg(m_lastEndToken).arg(node->start_token).arg(node->end_token));
        return;
      }

      Declaration* decl = instance.declaration;
      if(!decl && dynamic_cast<const IdentifiedType*>(type.data()))
        decl = dynamic_cast<const IdentifiedType*>(type.data())->declaration();

      if(type && !dynamic_cast<const IdentifiedType*>(type.data())) {
        //Non-identified types do not have declarations, and can be integer-, string-literals, etc.
      } else {
        m_builder->newUse(start, end, decl);
      }
    }

    virtual void problem(AST* node, const QString& str) {
      if(m_dumpProblems)
        Cpp::ExpressionVisitor::problem(node, str);
      else
        kDebug() << "problem";
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
