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

#ifndef KDEVPLATFORM_NAMESPACEALIASDECLARATION_H
#define KDEVPLATFORM_NAMESPACEALIASDECLARATION_H

#include "declaration.h"
#include "declarationdata.h"

namespace KDevelop
{
class KDEVPLATFORMLANGUAGE_EXPORT NamespaceAliasDeclarationData : public DeclarationData
{
public:
  NamespaceAliasDeclarationData() {}
  NamespaceAliasDeclarationData( const NamespaceAliasDeclarationData& rhs )
      : DeclarationData( rhs )
  {
    m_importIdentifier = rhs.m_importIdentifier;
  }
  IndexedQualifiedIdentifier m_importIdentifier; //The identifier that was imported
};
/**
 * A class which represents a "using namespace" statement, or a "namespace A = B" statement.
 *
 * This class is used by the duchain search process to transparently transform the search according to
 * namespace aliases and namespace imports.
 *
 * A namespace import declaration must have an identifier that equals globalImportIdentifier.
 *
 * If the identifier of the declaration does not equal globalImportIdentifier, then the declaration
 * represents a namespace alias, where the name of the alias equals the declaration. In that case,
 * the declaration is additionally added to the persistent symbol table with its real scope and globalAliasIdentifer
 * appended, to allow an efficient lookup.
 */
class KDEVPLATFORMLANGUAGE_EXPORT NamespaceAliasDeclaration : public Declaration
{
public:
  NamespaceAliasDeclaration(const NamespaceAliasDeclaration& rhs);
  NamespaceAliasDeclaration(const RangeInRevision& range, DUContext* context);
  NamespaceAliasDeclaration(NamespaceAliasDeclarationData& data);

  virtual ~NamespaceAliasDeclaration();
  ///A NamespaceAliasDeclaration cannot have a type, so setAbstractType does nothing here.
  virtual void setAbstractType(AbstractType::Ptr type);

  /**The identifier that was imported.*/
  QualifiedIdentifier importIdentifier() const;
  /**
    * The identifier must be absolute (Resolve it before setting it!)
    * Although the identifier is global, the explicitlyGlobal() member must not be set
    */
  void setImportIdentifier(const QualifiedIdentifier& id);

  virtual void setInSymbolTable(bool inSymbolTable);  
  
  enum {
    Identity = 13
  };
  
  typedef Declaration BaseClass;

  virtual QString toString() const;
private:
  void unregisterAliasIdentifier();
  void registerAliasIdentifier();
  virtual Declaration* clonePrivate() const;
  DUCHAIN_DECLARE_DATA(NamespaceAliasDeclaration)
};
}

#endif // KDEVPLATFORM_NAMESPACEALIASDECLARATION_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
