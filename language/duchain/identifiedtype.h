/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Adam Treat <treat@kde.org>
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

#ifndef IDENTIFIEDTYPE_H
#define IDENTIFIEDTYPE_H

#include <languageexport.h>
#include <identifier.h>

namespace KDevelop
{

class Declaration;

/**
 * An IdentifiedType is a type that has a declaration.
 * Example of an identified type:
 * - A class type
 * Example of not identified types:
 * - Pointer types(can point to identified types, but itself has no declaration)
 * - Reference types(the same)
 * */
class KDEVPLATFORMLANGUAGE_EXPORT IdentifiedType
{
public:
  IdentifiedType(const IdentifiedType& rhs);
  IdentifiedType();

  QualifiedIdentifier identifier() const;

  bool equals(const IdentifiedType* rhs) const;
  
  Declaration* declaration() const;
  /**
   * You should be careful when setting this, because it also changes the meaning of the declaration.
   * The logic is:
   * If a declaration has a set abstractType(), and that abstractType() has set the same declaration as declaration(),
   * then the declaration declares the type(thus it is a type-declaration, see Declaration::kind())
   * */
  void setDeclaration(Declaration* declaration);

  QString idMangled() const;

private:
  class IdentifiedTypePrivate* const d;
};

}

#endif

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
