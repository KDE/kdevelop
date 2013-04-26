/* This file is part of KDevelop
    Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
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

#ifndef KDEVPLATFORM_FORWARDDECLARATION_H
#define KDEVPLATFORM_FORWARDDECLARATION_H

#include "declaration.h"
#include "declarationdata.h"

namespace KDevelop
{
class KDEVPLATFORMLANGUAGE_EXPORT ForwardDeclarationData : public DeclarationData
{
public:
  ForwardDeclarationData() {
  }
  ForwardDeclarationData( const ForwardDeclarationData& rhs ) 
      : DeclarationData( rhs )
  {
  }
};
/**
 * Represents a forward declaration
 */
class KDEVPLATFORMLANGUAGE_EXPORT ForwardDeclaration : public Declaration
{
public:
  /**
   * Constructor.
   *
   * If \a context is in the symbol table, the declaration will automatically be added into the symbol table.
   *
   * \param url url of the document where this occurred
   * \param range range of the alias declaration's identifier
   * \param parentContext context in which this declaration occurred
   * */
  ForwardDeclaration(const RangeInRevision& range, DUContext* context);
  ForwardDeclaration(ForwardDeclarationData& data);

  ///Copy-constructor for cloning
  ForwardDeclaration(const ForwardDeclaration& rhs);

  /// Destructor.
  virtual ~ForwardDeclaration();

  virtual bool isForwardDeclaration() const;

  /**
   * Resolved the forward-declaration using the given import-trace.
   * The topcontext is needed for correct functionality, and may only be
   * zero when the declaration is resolved starting from the top-context
   * the forward-declaration is contained in.
   *
   * If this forward-declaration has a type assigned that is not derived from ForwardDeclarationType,
   * and that is derived from IdentifiedType, the declaration of that type is returned here.
   * */
  virtual Declaration* resolve(const TopDUContext* topContext) const;

  virtual DUContext * logicalInternalContext(const TopDUContext* topContext) const;

  virtual QString toString() const;

  enum {
    Identity = 10
  };
  
  typedef Declaration BaseClass;
  
private:
  virtual Declaration* clonePrivate() const;
  DUCHAIN_DECLARE_DATA(ForwardDeclaration)
};

}

#endif // KDEVPLATFORM_DECLARATION_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
