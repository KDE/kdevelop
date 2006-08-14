/* This file is part of KDevelop
   Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2006 Jakob Petsovits <jpetso@gmx.at>

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

#include "csharp_binder.h"

#ifdef DO_DEBUG
#include "kdebug.h"
#define KDEBUG(text)  kDebug() << text
#else
#define KDEBUG(text)  ;
#endif


namespace csharp
{

Binder::Binder(CodeModel *model, parser::token_stream_type *token_stream)
  : _M_model(model), _M_token_stream(token_stream), _M_decoder(_M_token_stream),
    _M_editContext(false)
{
}

Binder::~Binder()
{
}

void Binder::run(const KUrl &url, compilation_unit_ast *node)
{
  _M_currentFile = url.url();
  _M_globalNamespace = model()->globalNamespace();
  _M_currentAccess = access_policy::access_private;
  NamespaceDeclarationModelItem ns = static_cast<NamespaceDeclarationModelItem>(_M_globalNamespace);
  changeCurrentNamespace( ns );
  visit_node(node);
}


QStringList Binder::changeCurrentContext(const QStringList& context)
{
  QStringList old = _M_currentContext;
  _M_currentContext = context;
  return old;
}

bool Binder::changeEditContext(bool edit)
{
  bool old = _M_editContext;
  _M_editContext = edit;
  return old;
}

ScopeModelItem Binder::changeCurrentScope(ScopeModelItem scope)
{
  ScopeModelItem old = _M_currentScope;
  _M_currentScope = scope;
  return old;
}

NamespaceDeclarationModelItem Binder::changeCurrentNamespace(NamespaceDeclarationModelItem item)
{
  NamespaceDeclarationModelItem old = _M_currentNamespace;
  _M_currentNamespace = item;
  return old;
}

access_policy::access_policy_enum Binder::changeCurrentAccess(access_policy::access_policy_enum accessPolicy)
{
  access_policy::access_policy_enum old = accessPolicy;
  _M_currentAccess = accessPolicy;
  return old;
}



void Binder::visit_namespace_declaration(namespace_declaration_ast *node)
{
  KDEBUG(">>> 1.");
  KDEBUG(">>> _M_currentContext = " << _M_currentContext.join("."));
  QStringList namespaceContext = _M_currentContext;
  KDEBUG(">>> namespaceContext = " << namespaceContext.join("."));
  QStringList oldContext = changeCurrentContext(namespaceContext);
  KDEBUG(">>> oldContext = " << oldContext.join("."));
  bool oldEditContext = changeEditContext(true);
  KDEBUG(">>> oldEditContext = " << oldEditContext);
  KDEBUG(">>> _M_editContext = " << _M_editContext);

  visit_node(node->name);
  KDEBUG(">>> 2.");
  KDEBUG(">>> _M_currentContext = " << _M_currentContext.join("."));
  KDEBUG(">>> namespaceContext = " << namespaceContext.join("."));
  KDEBUG(">>> oldContext = " << oldContext.join("."));
  KDEBUG(">>> _M_editContext = " << _M_editContext);

  changeEditContext(oldEditContext);
  namespaceContext = changeCurrentContext(oldContext);
  KDEBUG(">>> 3.");
  KDEBUG(">>> _M_currentContext = " << _M_currentContext.join("."));
  KDEBUG(">>> namespaceContext = " << namespaceContext.join("."));
  KDEBUG(">>> oldContext = " << oldContext.join("."));
  KDEBUG(">>> _M_editContext = " << _M_editContext);

  setPositionAt(_M_currentNamespace, node);

  NamespaceDeclarationModelItem namespaceDeclaration =
    _M_currentNamespace->createNamespace(namespaceContext);

  oldContext = changeCurrentContext(namespaceContext);
  visit_node(node->body);
  namespaceContext = changeCurrentContext(oldContext);
  KDEBUG(">>> 4.");
  KDEBUG(">>> _M_currentContext = " << _M_currentContext.join("."));
  KDEBUG(">>> namespaceContext = " << namespaceContext.join("."));
  KDEBUG(">>> oldContext = " << oldContext.join("."));
  KDEBUG(">>> _M_editContext = " << _M_editContext);
}

void Binder::visit_qualified_identifier(qualified_identifier_ast *node)
{
  KDEBUG(">>> 1.5.");
  if (node->name_sequence)
  {
    const list_node<identifier_ast*> *__it = node->name_sequence->to_front(), *__end = __it;
    do
    {
      visit_node(__it->element);

      if (_M_editContext)
      {
        identifier_ast *name = __it->element;
        _M_currentContext.append(decode_string(name->ident).c_str());
        KDEBUG(">>> decode_string() = " << decode_string(name->ident).c_str());
        KDEBUG(">>> _M_currentContext = " << _M_currentContext.join("."));
      }

      __it = __it->next;
    }
    while (__it != __end);
  }
}

void Binder::visit_optional_modifiers(optional_modifiers_ast *node)
{
  changeCurrentAccess(node->access_policy);
  // TODO: change modifiers
}

std::string Binder::decode_string(std::size_t index) const
{
  return _M_decoder.decode_string(index);
}

void Binder::setPositionAt(_CodeModelItem *item, ast_node */*node*/)
{
  /*
  int startLine, startColumn;
  int endLine, endColumn;

  const parser::token_type &start_token =
    _M_token_stream->token(node->start_token);
  const parser::token_type &end_token =
    _M_token_stream->token(node->end_token);

  _M_lexer->positionAt(start_token.position,
                       &startLine, &startColumn, &fileName);
  _M_lexer->positionAt(end_token.position,
                       &endLine, &endColumn, &fileName);
  */

  item->setFileName(_M_currentFile);
  item->setStartPosition(KTextEditor::Cursor(0 /*startLine*/, 0 /*startColumn*/));
  item->setEndPosition(KTextEditor::Cursor(0 /*endLine*/, 0 /*endColumn*/));
}

} // end of namespace csharp

// kate: space-indent on; indent-width 2; replace-tabs on;
