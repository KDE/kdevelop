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

#ifndef USEBUILDER_H
#define USEBUILDER_H

#include "contextbuilder.h"
#include <kdevexport.h>

typedef ContextBuilder UseBuilderBase;

/**
 * A class which iterates the AST to extract uses of definitions.
 */
class KDEVCPPLANGUAGE_EXPORT UseBuilder: public UseBuilderBase
{
public:
  UseBuilder(ParseSession* session);
  UseBuilder(CppEditorIntegrator* editor);

  /**
   * Compile either a context-definition chain, or add uses to an existing
   * chain.
   *
   * \param includes contexts to reference from the top context.  The list may be changed by this function.
   */
  void buildUses(AST *node);

protected:
  virtual void openContext(DUContext* newContext);
  virtual void closeContext();

  virtual void visitPrimaryExpression (PrimaryExpressionAST*);
  virtual void visitMemInitializer(MemInitializerAST *);

private:
  /// Register a new use
  void newUse(NameAST* name);

  inline int& nextUseIndex() { return m_nextUseStack.top(); }
  QStack<int> m_nextUseStack;
};

#endif // USEBUILDER_H

// kate: indent-width 2;
