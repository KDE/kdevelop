/* This file is part of KDevelop
    Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>

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

#ifndef NAME_COMPILER_H
#define NAME_COMPILER_H

#include "default_visitor.h"
#include "identifier.h"

class TokenStream;

class NameCompiler: protected DefaultVisitor
{
public:
  NameCompiler(TokenStream *token_stream);

  void run(NameAST *node) { internal_run(node); }
  void run(UnqualifiedNameAST *node) { internal_run(node); }

  QString name() const { return _M_name.toString(); }
  QStringList qualifiedName() const { return _M_name.toStringList(); }

  const QualifiedIdentifier& identifier() const;

protected:
  virtual void visitUnqualifiedName(UnqualifiedNameAST *node);
  virtual void visitTemplateArgument(TemplateArgumentAST *node);

  void internal_run(AST *node);
  QString decode_operator(std::size_t index) const;

private:
  TokenStream *_M_token_stream;
  QualifiedIdentifier m_base;
  Identifier m_currentIdentifier;
  QualifiedIdentifier _M_name;
};

#endif // NAME_COMPILER_H

// kate: space-indent on; indent-width 2; replace-tabs on;
