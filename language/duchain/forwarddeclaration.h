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

#ifndef FORWARDDECLARATION_H
#define FORWARDDECLARATION_H

#include "language/duchain/declaration.h"

namespace KDevelop
{
class ForwardDeclarationPrivate;
/**
 * Represents a forward declaration
 */
class KDEVPLATFORMLANGUAGE_EXPORT ForwardDeclaration : public Declaration
{
public:
  ForwardDeclaration(const ForwardDeclaration& rhs);
  ForwardDeclaration(const HashedString& url, const SimpleRange& range, DUContext* context);
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

  virtual Declaration* clone() const;

  DUContext * logicalInternalContext(const TopDUContext* topContext) const;

  virtual QString toString() const;
  
private:
  Q_DECLARE_PRIVATE(ForwardDeclaration)
};

}

#endif // DECLARATION_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
