/* This file is part of KDevelop
    Copyright (C) 2006 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#ifndef TYPESYSTEM_H
#define TYPESYSTEM_H

#include <identifier.h>
#include <languageexport.h>
#include <QtCore/QSet>

#include <QtCore/QList>

#include <ksharedptr.h>

namespace KDevelop
{

class AbstractType;
class IntegralType;
class PointerType;
class ReferenceType;
class FunctionType;
class StructureType;
class ArrayType;

class TypeVisitor
{
public:
  virtual ~TypeVisitor () {};

  virtual bool preVisit (const AbstractType *) { return true; }
  virtual void postVisit (const AbstractType *) {}

  virtual void visit (const IntegralType *) {}

  virtual bool visit (const PointerType *) { return true; }
  virtual void endVisit (const PointerType *) {}

  virtual bool visit (const ReferenceType *) { return true; }
  virtual void endVisit (const ReferenceType *) {}

  virtual bool visit (const FunctionType *) { return true; }
  virtual void endVisit (const FunctionType *) {}

  virtual bool visit (const StructureType *) { return true; }
  virtual void endVisit (const StructureType *) {}

  virtual bool visit (const ArrayType *) { return true; }
  virtual void endVisit (const ArrayType *) {}
};

class KDEVPLATFORMLANGUAGE_EXPORT AbstractType : public KShared
{
public:
  typedef KSharedPtr<AbstractType> Ptr;

  AbstractType();
  virtual ~AbstractType ();

  inline void accept (TypeVisitor *v) const
  {
    if (v->preVisit (this))
      this->accept0 (v);

    v->postVisit (this);
  }

  static void acceptType (AbstractType::Ptr type, TypeVisitor *v)
  {
    if (! type)
      return;

    type->accept (v);
  }

  virtual QString toString() const = 0;

  virtual QString mangled() const;

  uint hash() const;

  enum WhichType {
    TypeAbstract,
    TypeIntegral,
    TypePointer,
    TypeReference,
    TypeFunction,
    TypeStructure,
    TypeArray
  };

  virtual WhichType whichType() const { return TypeAbstract; }

protected:
  virtual void accept0 (TypeVisitor *v) const = 0;

//  template <class T>
//  void deregister(T* that) { TypeSystem::self()->deregisterType(that); }

private:
  class AbstractTypePrivate* const d;
};

class KDEVPLATFORMLANGUAGE_EXPORT IntegralType: public AbstractType
{
public:
  typedef KSharedPtr<IntegralType> Ptr;

  IntegralType();
  IntegralType(const QString& name);
  ~IntegralType();

  const QString& name() const;

  void setName(const QString& name);

  bool operator == (const IntegralType &other) const;

  bool operator != (const IntegralType &other) const;

  virtual QString toString() const;

  //virtual uint hash() const;

  virtual WhichType whichType() const { return TypeIntegral; }

protected:
  virtual void accept0 (TypeVisitor *v) const
  { v->visit (this); }

private:
  class IntegralTypePrivate* const d;
};

class KDEVPLATFORMLANGUAGE_EXPORT PointerType: public AbstractType
{
public:
  typedef KSharedPtr<PointerType> Ptr;

  PointerType ();
  ~PointerType();

  bool operator != (const PointerType &other) const;
  bool operator == (const PointerType &other) const;
  void setBaseType(AbstractType::Ptr type);
  AbstractType::Ptr baseType () const;


  virtual QString toString() const;

  //virtual uint hash() const;

  virtual WhichType whichType() const { return TypePointer; }

protected:
  virtual void accept0 (TypeVisitor *v) const;

private:
  class PointerTypePrivate* const d;
};

class KDEVPLATFORMLANGUAGE_EXPORT ReferenceType: public AbstractType
{
public:
  typedef KSharedPtr<ReferenceType> Ptr;

  ReferenceType ();
  ~ReferenceType();
  const AbstractType::Ptr baseType () const;

  void setBaseType(AbstractType::Ptr baseType);

  bool operator == (const ReferenceType &other) const;

  bool operator != (const ReferenceType &other) const;

  virtual QString toString() const;

  //virtual uint hash() const;

  virtual WhichType whichType() const { return TypeReference; }

protected:
  virtual void accept0 (TypeVisitor *v) const;

private:
  class ReferenceTypePrivate* const d;
};

class KDEVPLATFORMLANGUAGE_EXPORT FunctionType : public AbstractType
{
public:
  typedef KSharedPtr<FunctionType> Ptr;

  FunctionType();
  ~FunctionType();

  const AbstractType::Ptr returnType () const;

  void setReturnType(AbstractType::Ptr returnType);

  const QList<AbstractType::Ptr>& arguments () const;

  void addArgument(AbstractType::Ptr argument);
  void removeArgument(AbstractType::Ptr argument);

  bool operator == (const FunctionType &other) const;

  bool operator != (const FunctionType &other) const;

  virtual QString toString() const;

  //virtual uint hash() const;

  virtual WhichType whichType() const { return TypeFunction; }

protected:
  virtual void accept0 (TypeVisitor *v) const;

private:
  class FunctionTypePrivate* const d;
};

class KDEVPLATFORMLANGUAGE_EXPORT StructureType : public AbstractType
{
public:
  StructureType();
  ~StructureType();
  typedef KSharedPtr<StructureType> Ptr;

  const QList<AbstractType::Ptr>& elements () const;

  bool operator == (const StructureType &other) const;

  bool operator != (const StructureType &other) const;

  virtual void addElement(AbstractType::Ptr element);
  void removeElement(AbstractType::Ptr element);


  virtual QString toString() const;

  //virtual uint hash() const;

  virtual WhichType whichType() const { return TypeStructure; }

protected:
  virtual void accept0 (TypeVisitor *v) const;

private:
  class StructureTypePrivate* const d;
};

class KDEVPLATFORMLANGUAGE_EXPORT ArrayType : public AbstractType
{
public:
  typedef KSharedPtr<ArrayType> Ptr;

  ArrayType();
  ~ArrayType();

  int dimension () const;

  void setDimension(int dimension);

  AbstractType::Ptr elementType () const;

  void setElementType(AbstractType::Ptr type);

  bool operator == (const ArrayType &other) const;

  bool operator != (const ArrayType &other) const;

  virtual QString toString() const;

  //virtual uint hash() const;

  virtual WhichType whichType() const { return TypeArray; }

protected:
  virtual void accept0 (TypeVisitor *v) const;

private:
  class ArrayTypePrivate* const d;
};

template <class T>
uint qHash(const KSharedPtr<T>& type) { return type->hash(); }

}

#endif // TYPESYSTEM_H

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
