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

#include "codemodel_finder.h"
#include "codemodel.h"

CodeModelFinder::CodeModelFinder(CodeModel *model, ParseSession* session)
  : _M_model(model),
    name_cc(session),
    _M_resolve_policy(ResolveItem)
{
}

CodeModelFinder::~CodeModelFinder()
{
}

ScopeModelItem CodeModelFinder::resolveScope(NameAST *name, ScopeModelItem scope)
{
  Q_ASSERT(scope != 0);

  ResolvePolicy saved_resolve_policy = _M_resolve_policy;
  _M_resolve_policy = ResolveScope;

  ScopeModelItem old = changeCurrentScope(scope);

  visit(name);
  ScopeModelItem result = _M_current_scope;

  changeCurrentScope(old); // restore

  _M_resolve_policy = saved_resolve_policy;

  return result;
}

ScopeModelItem CodeModelFinder::changeCurrentScope(ScopeModelItem scope)
{
  ScopeModelItem old = _M_current_scope;
  _M_current_scope = scope;
  return old;
}

void CodeModelFinder::visitName(NameAST *node)
{
  visitNodes(this, node->qualified_names);

  if (_M_resolve_policy == ResolveItem)
    visit(node->unqualified_name);
}

void CodeModelFinder::visitUnqualifiedName(UnqualifiedNameAST *node)
{
  if (!_M_current_scope)
    {
      // nothing to do
      return;
    }

  name_cc.run(node);
  QString id = name_cc.name();

  if (ClassModelItem klass = _M_current_scope->findClass(id))
    {
      _M_current_scope = klass;
    }
  else if (NamespaceModelItem parentNamespace = model_safe_cast<NamespaceModelItem>(_M_current_scope))
    {
      NamespaceModelItem ns = parentNamespace->findNamespace(id);
      _M_current_scope = model_static_cast<ScopeModelItem>(ns);
    }
  else if (FileModelItem file = model_safe_cast<FileModelItem>(_M_current_scope))
    {
      NamespaceModelItem ns = file->findNamespace(id);
      _M_current_scope = model_static_cast<ScopeModelItem>(ns);
    }
  }

// kate: space-indent on; indent-width 2; replace-tabs on;

