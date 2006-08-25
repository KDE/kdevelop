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

#include "definitionbuilder.h"

#include <ktexteditor/smartrange.h>

#include "cppeditorintegrator.h"
#include "name_compiler.h"
#include "definition.h"

using namespace KTextEditor;

DefinitionBuilder::DefinitionBuilder (ParseSession* session)
  : DefinitionBuilderBase(session)
{
}

DefinitionBuilder::DefinitionBuilder (CppEditorIntegrator* editor)
  : DefinitionBuilderBase(editor)
{
}

TopDUContext* DefinitionBuilder::buildDefinitions(const KUrl& url, AST *node, QList<DUContext*>* includes)
{
  return buildContexts(url, node, includes);
}

void DefinitionBuilder::visitDeclarator (DeclaratorAST* node)
{
  // Don't create a definition for a function
  if (node->parameter_declaration_clause) {
    switch (currentContext()->type()) {
      case DUContext::Global:
      case DUContext::Namespace:
      case DUContext::Class:
      case DUContext::Function:
          break;

      case DUContext::Other:
          newDeclaration(node->id, node);
          break;
    }

  } else {
    newDeclaration(node->id, node);
  }

  DefinitionBuilderBase::visitDeclarator(node);
}

Definition* DefinitionBuilder::newDeclaration(NameAST* name, AST* rangeNode)
{
  Definition::Scope scope = Definition::GlobalScope;
  switch (currentContext()->type()) {
    case DUContext::Namespace:
      scope = Definition::NamespaceScope;
      break;
    case DUContext::Class:
      scope = Definition::ClassScope;
      break;
    case DUContext::Function:
      scope = Definition::LocalScope;
      break;
    default:
      break;
  }

  Range* prior = m_editor->currentRange();
  Range* range = m_editor->createRange(name ? static_cast<AST*>(name) : rangeNode);
  m_editor->exitCurrentRange();
  Q_ASSERT(m_editor->currentRange() == prior);

  Definition* definition = new Definition(range, scope);
  currentContext()->addDefinition(definition);

  if (name) {
    m_nameCompiler->run(name);

    // FIXME this can happen if we're defining a staticly declared variable
    //Q_ASSERT(m_nameCompiler->identifier().count() == 1);
    Q_ASSERT(!m_nameCompiler->identifier().isEmpty());
    definition->setIdentifier(m_nameCompiler->identifier().first());
  }

  return definition;
}
