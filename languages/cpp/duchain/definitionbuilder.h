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

#ifndef DEFINITIONBUILDER_H
#define DEFINITIONBUILDER_H

#include "contextbuilder.h"

class Definition;

typedef ContextBuilder DefinitionBuilderBase;

/**
 * A class which iterates the AST to extract definitions of types.
 */
class DefinitionBuilder: public DefinitionBuilderBase
{
public:
  DefinitionBuilder(ParseSession* session);
  DefinitionBuilder(CppEditorIntegrator* editor);

  /**
   * Compile either a context-definition chain, or add uses to an existing
   * chain.
   *
   * \param includes contexts to reference from the top context.  The list may be changed by this function.
   */
  TopDUContext* buildDefinitions(const KUrl& url, AST *node, QList<DUContext*>* includes = 0);

protected:
  virtual void visitDeclarator (DeclaratorAST*);

private:
  /**
   * Register a new declaration with the definition-use chain.
   * Returns the new context created by this definition.
   * \param range provide a valid AST here if name is null
   */
  Definition* newDeclaration(NameAST* name, AST* range = 0);
};

#endif // DEFINITIONBUILDER_H

// kate: indent-width 2;
