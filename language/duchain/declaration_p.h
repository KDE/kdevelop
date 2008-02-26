/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#ifndef DECLARATION_P_H
#define DECLARATION_P_H

#include "duchainbase_p.h"
#include "contextowner_p.h"

#include "declaration.h"
#include "ducontext.h"
#include "ducontext_p.h"
#include "topducontext.h"
#include "duchainlock.h"
#include "duchain.h"

namespace KDevelop
{

class DeclarationPrivate : public DUChainBasePrivate
{
public:
  DeclarationPrivate();
  
  DeclarationPrivate( const DeclarationPrivate& rhs );
  
  DUContext* m_context, *m_internalContext;
  Declaration::Scope m_scope;
  AbstractType::Ptr m_type;
  Identifier m_identifier;
  
  QByteArray m_comment;

  Declaration::Kind m_kind;

  bool m_isDefinition  : 1;
  bool m_inSymbolTable : 1;
  bool m_isTypeAlias   : 1;
  bool m_anonymousInContext : 1; //Whether the declaration was added into the parent-context anonymously
  
};

}

#endif
