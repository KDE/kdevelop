/*
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef CONTEXTOWNER_H
#define CONTEXTOWNER_H

#include "../languageexport.h"

namespace KDevelop
{
  class DUContext;
  class Definition;
  class Declaration;
  class DUChainBase;

/**
 * This is a shared base-class for Declaration and Definition.
 * */
class KDEVPLATFORMLANGUAGE_EXPORT ContextOwner {
  public:
    ContextOwner(DUChainBase* self);
    virtual ~ContextOwner();
  /**
   * If this item opens an own context, this returns that context.
   * If this is a resolved forward-declaration, this returns the resolved declaration's internal context.
   * */
  virtual DUContext* internalContext() const;
  void setInternalContext(DUContext* context);

  ///If this is a declaration, this returns the pointer, else zero.
  Declaration* asDeclaration();

  ///If this is a definition, this returns the pointer, else zero.
  Definition* asDefinition();

  ///If this is a declaration, this returns the pointer, else zero.
  const Declaration* asDeclaration() const;

  ///If this is a definition, this returns the pointer, else zero.
  const Definition* asDefinition() const;

  private:
    class ContextOwnerPrivate* const d;
};
}

#endif
