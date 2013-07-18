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

#ifndef KDEVPLATFORM_TYPEUTILS_H
#define KDEVPLATFORM_TYPEUTILS_H

#include "../../languageexport.h"
#include "abstracttype.h"
namespace KDevelop {
  class TopDUContext;
}
namespace TypeUtils {
  /**
   * Returns the completely dereferenced and un-aliased type, pointers are also dereferenced(example: ReferenceType(PointerType(int)) -> int)
   * All modifiers are pushed from the aliases into the targets.
   *
   *  !!DU-Chain must be locked!
   * Modifiers of aliases ore references are pushed into the targets.
   * @return return-value will only be zero if type is zero
   */
  KDEVPLATFORMLANGUAGE_EXPORT KDevelop::AbstractType::Ptr targetType(const KDevelop::AbstractType::Ptr& type, const KDevelop::TopDUContext* topContext, bool* constant = 0);
  /**
   * Same as targetType(..), ecept that it does not un-aliases TypeAliasTypes
   * Modifiers of aliases ore references are pushed into the targets.
   */
  KDEVPLATFORMLANGUAGE_EXPORT KDevelop::AbstractType::Ptr targetTypeKeepAliases(const KDevelop::AbstractType::Ptr& type, const KDevelop::TopDUContext* topContext, bool* constant = 0);
  /**
   * Resolves all type-aliases, returning the effective aliased type
   * All modifiers are pushed from the aliases into the targets.
   */
  KDEVPLATFORMLANGUAGE_EXPORT KDevelop::AbstractType::Ptr unAliasedType(const KDevelop::AbstractType::Ptr& type);
}

#endif
