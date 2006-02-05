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

#ifndef BINDER_H
#define BINDER_H

#include "default_visitor.h"
#include "codemodel.h"
#include "type_compiler.h"
#include "name_compiler.h"
#include "declarator_compiler.h"

#include <QtCore/QSet>

#include <kurl.h>

class Lexer;
class TokenStream;
struct NameSymbol;

class Binder: protected DefaultVisitor
{
public:
  Binder(CodeModel *model,
         TokenStream *token_stream,
         Lexer *lexer);
  virtual ~Binder();

  void run(const KUrl &url, AST *node);

protected:
  virtual void visitAccessSpecifier(AccessSpecifierAST *);
  virtual void visitClassSpecifier(ClassSpecifierAST *);
  virtual void visitEnumSpecifier(EnumSpecifierAST *);
  virtual void visitEnumerator(EnumeratorAST *);
  virtual void visitFunctionDefinition(FunctionDefinitionAST *);
  virtual void visitLinkageSpecification(LinkageSpecificationAST *);
  virtual void visitNamespace(NamespaceAST *);
  virtual void visitSimpleDeclaration(SimpleDeclarationAST *);
  virtual void visitTemplateDeclaration(TemplateDeclarationAST *);
  virtual void visitTypedef(TypedefAST *);
  virtual void visitUsing(UsingAST *);
  virtual void visitUsingDirective(UsingDirectiveAST *);

  inline CodeModel *model() const { return _M_model; }

private:
  TypeInfo qualifyType(const TypeInfo &type, const QStringList &context) const;

  int decode_token(std::size_t index) const;
  const NameSymbol *decode_symbol(std::size_t index) const;
  CodeModel::AccessPolicy decode_access_policy(std::size_t index) const;
  CodeModel::ClassType decode_class_type(std::size_t index) const;

  CodeModel::AccessPolicy changeCurrentAccess(CodeModel::AccessPolicy accessPolicy);
  NamespaceModelItem changeCurrentNamespace(NamespaceModelItem item);
  ClassModelItem changeCurrentClass(ClassModelItem item);
  FunctionDefinitionModelItem changeCurrentFunction(FunctionDefinitionModelItem item);
  TemplateModelItem changeCurrentTemplate(TemplateModelItem item);

  void declare_symbol(SimpleDeclarationAST *node, InitDeclaratorAST *init_declarator);

  ScopeModelItem currentScope();

  void applyStorageSpecifiers(const ListNode<std::size_t> *storage_specifiers, MemberModelItem item);
  void applyFunctionSpecifiers(const ListNode<std::size_t> *it, FunctionModelItem item);

  void setPositionAt(_CodeModelItem *item, AST *ast);

private:
  CodeModel *_M_model;
  TokenStream *_M_token_stream;
  Lexer *_M_lexer;
  QString _M_currentFile;

  CodeModel::AccessPolicy _M_current_access;
  FileModelItem _M_current_file;
  NamespaceModelItem _M_current_namespace;
  NamespaceModelItem _M_global_namespace;
  ClassModelItem _M_current_class;
  FunctionDefinitionModelItem _M_current_function;
  TemplateModelItem _M_current_template;
  EnumModelItem _M_current_enum;

  QStringList _M_context;
  QSet<QString> _M_qualified_types;

protected:
  TypeCompiler type_cc;
  NameCompiler name_cc;
  DeclaratorCompiler decl_cc;
};

#endif // BINDER_H

// kate: space-indent on; indent-width 2; replace-tabs on;
