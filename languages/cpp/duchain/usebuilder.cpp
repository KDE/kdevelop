/* This file is part of KDevelop
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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
// kate: indent-width 2;

#include "usebuilder.h"

#include <ktexteditor/smartrange.h>
#include <ktexteditor/smartinterface.h>

#include "cppeditorintegrator.h"
#include "name_compiler.h"
#include "declaration.h"
#include "use.h"
#include "topducontext.h"
#include "duchain.h"

using namespace KTextEditor;

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
  Range newRange = m_editor->findRange(name);

  QualifiedIdentifier id = identifierForName(name);

  QReadLocker readLock(DUChain::lock());
  QList<Declaration*> declarations = currentContext()->findDeclarations(id, newRange.start());
  foreach (Declaration* declaration, declarations)
    if (!declaration->isForwardDeclaration()) {
      declarations.clear();
      declarations.append(declaration);
      break;
    }
  // If we don't break, there's no non-forward declaration

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

      if (use->textRange().start() > translated.end())
        break;

      if (use->textRange() == translated &&
          ((!use->declaration() && declarations.isEmpty()) ||
           (declarations.count() == 1 && use->declaration() == declarations.first())))
      {
        // Match
        ret = use;
        break;
      }
    }
  }

  if (!ret) {
    readLock.unlock();
    QWriteLocker lock(DUChain::lock());

    Range* prior = m_editor->currentRange();
    Range* use = m_editor->createRange(newRange);
    m_editor->exitCurrentRange();
    Q_ASSERT(m_editor->currentRange() == prior);

    Use* newUse = new Use(use, currentContext());

    if (declarations.count())
      declarations.first()->addUse(newUse);
    else
      currentContext()->addOrphanUse(newUse);
      //kWarning() << k_funcinfo << "Could not find definition for identifier " << id << " at " << *use << endl;
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
