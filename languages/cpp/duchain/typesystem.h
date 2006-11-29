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

#include "identifier.h"
#include <kdevexport.h>
#include <QSet>
#include <QString>
#include <QList>

#include <ksharedptr.h>

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

class KDEVCPPLANGUAGE_EXPORT AbstractType : public KShared
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

  virtual QString mangled() const { return QString(); }

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
  bool m_registered;
};

class IntegralType: public AbstractType
{
public:
  typedef KSharedPtr<IntegralType> Ptr;

  IntegralType();
  IntegralType(const QString& name);

  inline const QString& name() const
  { return m_name; }

  inline void setName(const QString& name)
  { m_name = name; }

  inline bool operator == (const IntegralType &other) const
  { return m_name == other.m_name; }

  inline bool operator != (const IntegralType &other) const
  { return m_name != other.m_name; }

  virtual QString toString() const { return m_name; }

  //virtual uint hash() const;

  virtual WhichType whichType() const { return TypeIntegral; }

protected:
  virtual void accept0 (TypeVisitor *v) const
  { v->visit (this); }

private:
  QString m_name;
};

class PointerType: public AbstractType
{
public:
  typedef KSharedPtr<PointerType> Ptr;

  PointerType ();

  inline AbstractType::Ptr baseType () const
  { return m_baseType; }

  inline void setBaseType(AbstractType::Ptr type)
  { m_baseType = type; }

  inline bool operator == (const PointerType &other) const
  { return m_baseType == other.m_baseType; }

  inline bool operator != (const PointerType &other) const
  { return m_baseType != other.m_baseType; }

  virtual QString toString() const;

  //virtual uint hash() const;

  virtual WhichType whichType() const { return TypePointer; }

protected:
  virtual void accept0 (TypeVisitor *v) const
  {
    if (v->visit (this))
      acceptType (m_baseType, v);

    v->endVisit (this);
  }

private:
  AbstractType::Ptr m_baseType;
};

class ReferenceType: public AbstractType
{
public:
  typedef KSharedPtr<ReferenceType> Ptr;

  ReferenceType ();

  inline const AbstractType::Ptr baseType () const
  { return m_baseType; }

  inline void setBaseType(AbstractType::Ptr baseType)
  { m_baseType = baseType; }

  inline bool operator == (const ReferenceType &other) const
  { return m_baseType == other.m_baseType; }

  inline bool operator != (const ReferenceType &other) const
  { return m_baseType != other.m_baseType; }

  virtual QString toString() const;

  //virtual uint hash() const;

  virtual WhichType whichType() const { return TypeReference; }

protected:
  virtual void accept0 (TypeVisitor *v) const
  {
    if (v->visit (this))
      acceptType (m_baseType, v);

    v->endVisit (this);
  }

private:
  AbstractType::Ptr m_baseType;
};

class FunctionType : public AbstractType
{
public:
  typedef KSharedPtr<FunctionType> Ptr;

  FunctionType();

  inline const AbstractType::Ptr returnType () const
  { return m_returnType; }

  void setReturnType(AbstractType::Ptr returnType);

  inline const QList<AbstractType::Ptr>& arguments () const
  { return m_arguments; }

  void addArgument(AbstractType::Ptr argument);
  void removeArgument(AbstractType::Ptr argument);

  inline bool operator == (const FunctionType &other) const
  { return m_returnType == other.m_returnType && m_arguments == other.m_arguments; }

  inline bool operator != (const FunctionType &other) const
  { return m_returnType != other.m_returnType || m_arguments != other.m_arguments; }

  virtual QString toString() const;

  //virtual uint hash() const;

  virtual WhichType whichType() const { return TypeFunction; }

protected:
  virtual void accept0 (TypeVisitor *v) const
  {
    if (v->visit (this))
      {
        acceptType (m_returnType, v);

        for (int i = 0; i < m_arguments.count (); ++i)
          acceptType (m_arguments.at (i), v);
      }

    v->endVisit (this);
  }

private:
  AbstractType::Ptr m_returnType;
  QList<AbstractType::Ptr> m_arguments;
};

class StructureType : public AbstractType
{
public:
  typedef KSharedPtr<StructureType> Ptr;

  inline const QList<AbstractType::Ptr>& elements () const
  { return m_elements; }

  virtual void addElement(AbstractType::Ptr element);
  void removeElement(AbstractType::Ptr element);

  inline bool operator == (const StructureType &other) const
  { return m_elements == other.m_elements; }

  inline bool operator != (const StructureType &other) const
  { return m_elements != other.m_elements; }

  virtual QString toString() const;

  //virtual uint hash() const;

  virtual WhichType whichType() const { return TypeStructure; }

protected:
  virtual void accept0 (TypeVisitor *v) const
  {
    if (v->visit (this))
      {
        for (int i = 0; i < m_elements.count (); ++i)
          acceptType (m_elements.at (i), v);
      }

    v->endVisit (this);
  }

private:
  QList<AbstractType::Ptr> m_elements;
};

class ArrayType : public AbstractType
{
public:
  typedef KSharedPtr<ArrayType> Ptr;

  inline int dimension () const
  { return m_dimension; }

  inline void setDimension(int dimension)
  { m_dimension = dimension; }

  inline AbstractType::Ptr elementType () const
  { return m_elementType; }

  inline void setElementType(AbstractType::Ptr type)
  { m_elementType = type; }

  inline bool operator == (const ArrayType &other) const
  { return m_elementType == other.m_elementType && m_dimension == other.m_dimension; }

  inline bool operator != (const ArrayType &other) const
  { return m_elementType != other.m_elementType || m_dimension != other.m_dimension; }

  virtual QString toString() const;

  //virtual uint hash() const;

  virtual WhichType whichType() const { return TypeArray; }

protected:
  virtual void accept0 (TypeVisitor *v) const
  {
    if (v->visit (this))
      {
        acceptType (m_elementType, v);
      }

    v->endVisit (this);
  }

private:
  int m_dimension;
  AbstractType::Ptr m_elementType;
};

template <class T>
uint qHash(const KSharedPtr<T>& type) { return type->hash(); }

#endif // TYPESYSTEM_H
