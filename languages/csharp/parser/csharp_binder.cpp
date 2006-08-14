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


namespace csharp
{

Binder::Binder(CodeModel *model, parser::token_stream_type *token_stream)
  : _M_model(model), _M_token_stream(token_stream), _M_decoder(_M_token_stream)
{
}

Binder::~Binder()
{
}

void Binder::run(const KUrl &url, compilation_unit_ast *node)
{
  _M_currentFile = url.url();

  _M_currentAccessPolicy = access_policy::access_private;
  _M_currentModifiers = 0;

  changeCurrentScope( model_static_cast<ScopeModelItem>(model()->globalNamespace()) );
  visit_node(node);
}


ScopeModelItem Binder::changeCurrentScope(ScopeModelItem item)
{
  ScopeModelItem old = _M_currentScope;
  _M_currentScope = item;
  return old;
}

access_policy::access_policy_enum Binder::changeCurrentAccessPolicy(access_policy::access_policy_enum accessPolicy)
{
  access_policy::access_policy_enum old = accessPolicy;
  _M_currentAccessPolicy = accessPolicy;
  return old;
}

uint Binder::changeCurrentModifiers(uint modifiers)
{
  uint old = modifiers;
  _M_currentModifiers = modifiers;
  return old;
}


void Binder::visit_namespace_declaration(namespace_declaration_ast *node)
{
  NamespaceDeclarationModelItem newNamespaceDeclaration
    = model()->create<NamespaceDeclarationModelItem>();
  newNamespaceDeclaration->setScope( _M_currentScope->context() );
  setPositionAt( newNamespaceDeclaration, node );

  _M_additionalContext.clear();
  visit_node( node->name );

  NamespaceDeclarationModelItem parentNamespace =
    model_static_cast<NamespaceDeclarationModelItem>( _M_currentScope );

  while (_M_additionalContext.count() > 1)
    {
      QString firstName = _M_additionalContext.takeFirst();
      NamespaceDeclarationModelItem currentNamespace =
        parentNamespace->findNamespace( firstName );

      if (!currentNamespace)
        {
          currentNamespace = model()->create<NamespaceDeclarationModelItem>();
          currentNamespace->setScope( parentNamespace->context() );
          currentNamespace->setName( firstName );
          setPositionAt( currentNamespace, node );
          parentNamespace->addNamespace( currentNamespace );
        }
      parentNamespace = currentNamespace;
    }

  // set up the originally created namespace with the last remaining context name
  newNamespaceDeclaration->setScope( parentNamespace->context() );
  newNamespaceDeclaration->setName( _M_additionalContext.takeFirst() );
  parentNamespace->addNamespace( newNamespaceDeclaration );

  ScopeModelItem oldScope =
    changeCurrentScope( model_static_cast<ScopeModelItem>(newNamespaceDeclaration) );
  visit_node( node->body );
  changeCurrentScope( oldScope );
}

void Binder::visit_qualified_identifier(qualified_identifier_ast *node)
{
  if (node->name_sequence)
  {
    const list_node<identifier_ast*> *__it = node->name_sequence->to_front(), *__end = __it;
    do
    {
      visit_node(__it->element);

      identifier_ast *name = __it->element;
      _M_additionalContext.append( decode_string(name->ident).c_str() );

      __it = __it->next;
    }
    while (__it != __end);
  }
}

void Binder::visit_class_declaration(class_declaration_ast *node)
{
  ClassDeclarationModelItem newClassDeclaration
    = model()->create<ClassDeclarationModelItem>();
  newClassDeclaration->setScope( _M_currentScope->context() );
  setPositionAt( newClassDeclaration, node );

  _M_currentAttributes.clear();
  visit_node( node->attributes );

  visit_node( node->modifiers );

  visit_node( node->class_name );

  visit_node( node->type_parameters );

  visit_node( node->class_base );

  if (node->type_parameter_constraints_sequence)
  {
    const list_node<type_parameter_constraints_clause_ast*> *__it =
      node->type_parameter_constraints_sequence->to_front(), *__end = __it;
    do
    {
      visit_node( __it->element );
      __it = __it->next;
    }
    while (__it != __end);
  }

  newClassDeclaration->setName( decode_string(node->class_name->ident).c_str() );
  newClassDeclaration->setAccessPolicy( _M_currentAccessPolicy );
  newClassDeclaration->setPartial( node->partial );
  newClassDeclaration->setNew( _M_currentModifiers & modifiers::mod_new );
  newClassDeclaration->setUnsafe( _M_currentModifiers & modifiers::mod_unsafe );
  newClassDeclaration->setSealed( _M_currentModifiers & modifiers::mod_sealed );
  newClassDeclaration->setAbstract( _M_currentModifiers & modifiers::mod_abstract );
  newClassDeclaration->setStatic( _M_currentModifiers & modifiers::mod_static );

  if (NamespaceDeclarationModelItem ns = model_dynamic_cast<NamespaceDeclarationModelItem>(_M_currentScope))
    ns->addClass(newClassDeclaration);
  else if (ClassLikeDeclarationModelItem cl = model_dynamic_cast<ClassLikeDeclarationModelItem>(_M_currentScope))
    cl->addClass(newClassDeclaration);

  ScopeModelItem oldScope =
    changeCurrentScope( model_static_cast<ScopeModelItem>(newClassDeclaration) );
  visit_node( node->body );
  changeCurrentScope( oldScope );
}

void Binder::visit_optional_modifiers(optional_modifiers_ast *node)
{
  _M_currentAccessPolicy = node->access_policy;
  _M_currentModifiers = node->modifiers;
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
