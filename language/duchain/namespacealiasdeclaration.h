/* This file is part of KDevelop
    Copyright 2007 David Nolden<david.nolden.kdevelop@art-master.de>

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

#ifndef NAMESPACEALIASDECLARATION_H
#define NAMESPACEALIASDECLARATION_H

#include "declaration.h"
#include "../languageexport.h"

namespace KDevelop
{
/**
 * A class which represents a "using namespace" statement, or a "namespace A = B" statement.
 *
 *
 */
class KDEVPLATFORMLANGUAGE_EXPORT NamespaceAliasDeclaration : public Declaration
{
public:
  NamespaceAliasDeclaration(const NamespaceAliasDeclaration& rhs);
  NamespaceAliasDeclaration(const HashedString& url, KTextEditor::Range* range, Scope scope, DUContext* context);
  virtual ~NamespaceAliasDeclaration();

  ///A NamespaceAliasDeclaration cannot have a type, so setAbstractType does nothing here.
  virtual void setAbstractType(AbstractType::Ptr type);

  virtual Declaration* clone() const;

  /**The identifier that was imported.*/
  QualifiedIdentifier importIdentifier() const;
  ///The identifier must be absolute(Resolve it before setting it!) Since it is absolute, it should also be explicitlyGlobal.
  void setImportIdentifier(const QualifiedIdentifier& id);

  //Declaration::identifier() is the identifier that importIdentifier() should be "renamed to" within scope, or globalImportIdentifier() if it should be imported.

  virtual QString toString() const;
private:
  class NamespaceAliasDeclarationPrivate* const d;
};
}

#endif // FUNCTIONDECLARATION_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
