/* This file is part of KDevelop
    Copyright 2008 David Nolden<david.nolden.kdevelop@art-master.de>

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

#ifndef ALIASDECLARATION_H
#define ALIASDECLARATION_H

#include "declaration.h"
#include "duchainpointer.h"

namespace KDevelop
{
class AliasDeclarationPrivate;
/**
 * An alias declaration maps one declaration to another.
 * While searching in the duchain, an AliasDeclaration is transparently
 * replaced by its aliased declaration.
 */
class KDEVPLATFORMLANGUAGE_EXPORT AliasDeclaration : public Declaration
{
public:
  AliasDeclaration(const AliasDeclaration& rhs);
  //Constructs a AliasDeclaration. The default value for isNamespaceAlias is true.
  AliasDeclaration(const HashedString& url, const SimpleRange& range, DUContext* context);
  virtual ~AliasDeclaration();

  ///A AliasDeclaration cannot have a type, so setAbstractType does nothing here.
  virtual void setAbstractType(AbstractType::Ptr type);

  virtual Declaration* clone() const;

  void setAliasedDeclaration(const DeclarationPointer& decl);
  DeclarationPointer aliasedDeclaration() const;

  virtual QString toString() const;
private:
  Q_DECLARE_PRIVATE(AliasDeclaration)
};
}

#endif // FUNCTIONDECLARATION_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
