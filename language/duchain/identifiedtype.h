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

#include "language/duchain/identifier.h"
#include "declarationid.h"

namespace KDevelop
{

class Declaration;
class DeclarationId;
class TopDUContext;

class IdentifiedTypeData
{
public:
  DeclarationId m_id;
};

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
  virtual ~IdentifiedType();

//   QualifiedIdentifier identifier() const;

  bool equals(const IdentifiedType* rhs) const;

  void clear();
  
  //This is relatively expensive. Use declarationId instead when possible!
  QualifiedIdentifier qualifiedIdentifier() const;
  
  uint hash() const;

  DeclarationId declarationId() const;
  void setDeclarationId(const DeclarationId& id);
  
  Declaration* declaration(const TopDUContext* top) const;
  /**
   * You should be careful when setting this, because it also changes the meaning of the declaration.
   * The logic is:
   * If a declaration has a set abstractType(), and that abstractType() has set the same declaration as declaration(),
   * then the declaration declares the type(thus it is a type-declaration, see Declaration::kind())
   * */
  void setDeclaration(Declaration* declaration);

//   QString idMangled() const;
  virtual IdentifiedTypeData* idData() = 0;
  virtual const IdentifiedTypeData* idData() const = 0;
};

///Implements everything necessary to merge the given Parent class with IdentifiedType
///Your used Data class must be based on the Data member class
template<class Parent>
class KDEVPLATFORMLANGUAGE_EXPORT MergeIdentifiedType : public Parent, public IdentifiedType {
  public:
    
    class Data : public Parent::Data, public IdentifiedTypeData {
    };
    
    MergeIdentifiedType(Data& data) : Parent(data) {
    }
    
    virtual IdentifiedTypeData* idData() {
      return static_cast<Data*>(this->d_func_dynamic());
    }
    
    virtual const IdentifiedTypeData* idData() const {
      return static_cast<const Data*>(this->d_func());
    }
};

}

#endif

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
