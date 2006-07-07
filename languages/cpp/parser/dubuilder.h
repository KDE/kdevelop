/* This file is part of KDevelop
    Copyright (C) 2006 Roberto Raggi <roberto@kdevelop.org>
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

#ifndef DUBUILDER_H
#define DUBUILDER_H

#include "default_visitor.h"

class TokenStream;
class DUChain;
class DUContext;
class EditorIntegrator;
class NameCompiler;
class TypeEnvironment;

/**
 * A class which iterates the AST to extract definitions of types and their uses.
 *
 * This is the second pass of processing an AST.  The first is the type binder.
 *
 * \sa Binder
 */
class DUBuilder: protected DefaultVisitor
{
public:
  DUBuilder (TokenStream *token_stream, DUChain* chain);
  virtual ~DUBuilder ();

  void operator () (AST *node);

protected:
  virtual void visitNamespace (NamespaceAST *);
  virtual void visitClassSpecifier (ClassSpecifierAST *);
  virtual void visitTemplateDeclaration (TemplateDeclarationAST *);
  virtual void visitTypedef (TypedefAST *);
  virtual void visitFunctionDefinition (FunctionDefinitionAST *);
  virtual void visitParameterDeclarationClause (ParameterDeclarationClauseAST *);
  virtual void visitParameterDeclaration (ParameterDeclarationAST *);
  virtual void visitCompoundStatement (CompoundStatementAST *);
  virtual void visitSimpleDeclaration (SimpleDeclarationAST *);
  virtual void visitName (NameAST *);

  inline bool inNamespace (bool f) {
    bool was = in_namespace;
    in_namespace = f;
    return was;
  }

  inline bool inClass (bool f) {
    bool was = in_class;
    in_class = f;
    return was;
  }

  inline bool inTemplateDeclaration (bool f) {
    bool was = in_template_declaration;
    in_template_declaration = f;
    return was;
  }

  inline bool inTypedef (bool f) {
    bool was = in_typedef;
    in_typedef = f;
    return was;
  }

  inline bool inFunctionDefinition (bool f) {
    bool was = in_function_definition;
    in_function_definition = f;
    return was;
  }

  inline bool inParameterDeclaration (bool f) {
    bool was = in_parameter_declaration;
    in_parameter_declaration = f;
    return was;
  }

private:
  /**
   * Register a new declaration with the definition-use chain.
   * Returns the new context created by this definition.
   */
  DUContext* newDeclaration(TypeSpecifierAST* type);

  void closeContext(AST* node, DUContext* parent);

  TokenStream *_M_token_stream;
  EditorIntegrator* m_editor;

  NameCompiler* m_nameCompiler;

  bool in_namespace: 1;
  bool in_class: 1;
  bool in_template_declaration: 1;
  bool in_typedef: 1;
  bool in_function_definition: 1;
  bool in_parameter_declaration: 1;

  DUChain* m_chain;
  DUContext* m_currentContext;
  TypeEnvironment* m_types;
};

#endif // DUBUILDER_H

// kate: indent-width 2;
