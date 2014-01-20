/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>
    Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef KDEVPLATFORM_DECLARATION_DATA_H
#define KDEVPLATFORM_DECLARATION_DATA_H

#include "duchainbase.h"

#include "declaration.h"
#include "declarationid.h"
#include "ducontext.h"
#include "topducontext.h"
#include "duchainlock.h"
#include "duchain.h"
#include "../languageexport.h"
#include "types/indexedtype.h"

namespace KDevelop
{

class KDEVPLATFORMLANGUAGE_EXPORT DeclarationData : public DUChainBaseData
{
public:
  DeclarationData();

  DeclarationData( const DeclarationData& rhs );

  IndexedDUContext m_internalContext;
  IndexedType m_type;
  IndexedIdentifier m_identifier;

  ///@todo Eventually move this and all the definition/declaration coupling functionality somewhere else
  //Holds the declaration id for this definition, if this is a definition with separate declaration
  DeclarationId m_declaration;

  //Index in the comment repository
  uint m_comment;

  Declaration::Kind m_kind;

  bool m_isDefinition  : 1;
  bool m_inSymbolTable : 1;
  bool m_isTypeAlias   : 1;
  bool m_anonymousInContext : 1; //Whether the declaration was added into the parent-context anonymously
  bool m_isDeprecated      : 1;
  bool m_alwaysForceDirect : 1;
  bool m_isAutoDeclaration : 1;
  bool m_isExplicitlyDeleted : 1;
};
}

#endif
