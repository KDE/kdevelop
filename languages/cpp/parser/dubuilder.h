/* This file is part of KDevelop
    Copyright (C) 2006 Roberto Raggi <roberto@kdevelop.org>

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

class DUBuilder: protected DefaultVisitor
{
public:
  DUBuilder (TokenStream *token_stream);
  virtual ~DUBuilder ();

  void operator () (AST *node);

protected:
  virtual void visitNamespace (NamespaceAST *);
  virtual void visitClassSpecifier (ClassSpecifierAST *);
  virtual void visitTemplateDeclaration (TemplateDeclarationAST *);
  virtual void visitTypedef (TypedefAST *);
  virtual void visitFunctionDefinition (FunctionDefinitionAST *);
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

private:
  TokenStream *_M_token_stream;

  union {
    struct {
      uint in_namespace: 1;
      uint in_class: 1;
      uint in_template_declaration: 1;
      uint in_typedef: 1;
      uint in_function_definition: 1;
    };

    uint _M_flags;
  };
};

#endif // DUBUILDER_H

// kate: indent-width 2;
