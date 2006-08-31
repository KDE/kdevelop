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

#include "cppeditorintegrator.h"
#include "name_compiler.h"
#include "declaration.h"
#include "use.h"
#include "topducontext.h"

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
  Range* current = m_editor->currentRange();
  Range* use = m_editor->createRange(name);
  m_editor->exitCurrentRange();
  Q_ASSERT(m_editor->currentRange() == current);

  if (use->isSmartRange())
    if (use->toSmartRange()->parentRange() != currentContext()->smartRange())
      kWarning() << k_funcinfo << "Use " << *use << " parent " << *use->toSmartRange()->parentRange() << " " << use->toSmartRange()->parentRange() << " != current context " << *currentContext()->smartRange() << " " << currentContext()->smartRange() << " id " << currentContext()->scopeIdentifier() << endl;

  QualifiedIdentifier id = identifierForName(name);

  if (Declaration* definition = currentContext()->findDeclaration(id, use->start()))
    definition->addUse(new Use(use, currentContext()));
  else
    currentContext()->addOrphanUse(new Use(use, currentContext()));
    //kWarning() << k_funcinfo << "Could not find definition for identifier " << id << " at " << *use << endl;
}
