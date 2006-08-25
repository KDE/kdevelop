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

#include <QSet>
#include <QString>
#include <QList>

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
  virtual ~TypeVisitor ();

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

class AbstractType
{
public:
  virtual ~AbstractType () {}

  inline void accept (TypeVisitor *v) const
  {
    if (v->preVisit (this))
      this->accept0 (v);

    v->postVisit (this);
  }

  static void acceptType (const AbstractType *type, TypeVisitor *v)
  {
    if (! type)
      return;

    type->accept (v);
  }

protected:
  virtual void accept0 (TypeVisitor *v) const = 0;
};

class IntegralType: public AbstractType
{
public:
  inline const QString& name() const
  { return m_name; }

  inline void setName(const QString& name)
  { m_name = name; }

  inline bool operator == (const IntegralType &other) const
  { return m_name == other.m_name; }

  inline bool operator != (const IntegralType &other) const
  { return m_name != other.m_name; }

protected:
  virtual void accept0 (TypeVisitor *v) const
  { v->visit (this); }

private:
  QString m_name;
};

class PointerType: public AbstractType
{
public:
  PointerType ();

  inline AbstractType *baseType () const
  { return m_baseType; }

  inline void setBaseType(AbstractType* type)
  { m_baseType = type; }

  inline bool operator == (const PointerType &other) const
  { return m_baseType == other.m_baseType; }

  inline bool operator != (const PointerType &other) const
  { return m_baseType != other.m_baseType; }

protected:
  virtual void accept0 (TypeVisitor *v) const
  {
    if (v->visit (this))
      acceptType (m_baseType, v);

    v->endVisit (this);
  }

private:
  AbstractType *m_baseType;
};

class ReferenceType: public AbstractType
{
public:
  ReferenceType ();

  inline const AbstractType *baseType () const
  { return m_baseType; }

  inline void setBaseType(AbstractType *baseType)
  { m_baseType = baseType; }

  inline bool operator == (const ReferenceType &other) const
  { return m_baseType == other.m_baseType; }

  inline bool operator != (const ReferenceType &other) const
  { return m_baseType != other.m_baseType; }

protected:
  virtual void accept0 (TypeVisitor *v) const
  {
    if (v->visit (this))
      acceptType (m_baseType, v);

    v->endVisit (this);
  }

private:
  AbstractType *m_baseType;
};

class FunctionType : public AbstractType
{
public:
  FunctionType();

  inline const AbstractType *returnType () const
  { return m_returnType; }

  void setReturnType(AbstractType *returnType);

  inline const QList<AbstractType *>& arguments () const
  { return m_arguments; }

  void addArgument(AbstractType *argument);
  void removeArgument(AbstractType *argument);

  inline bool operator == (const FunctionType &other) const
  { return m_returnType == other.m_returnType && m_arguments == other.m_arguments; }

  inline bool operator != (const FunctionType &other) const
  { return m_returnType != other.m_returnType || m_arguments != other.m_arguments; }

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
  AbstractType *m_returnType;
  QList<AbstractType *> m_arguments;
};

class StructureType : public AbstractType
{
public:
  inline const QList<AbstractType *>& elements () const
  { return m_elements; }

  void addElement(AbstractType *element);
  void removeElement(AbstractType *element);

  inline bool operator == (const StructureType &other) const
  { return m_elements == other.m_elements; }

  inline bool operator != (const StructureType &other) const
  { return m_elements != other.m_elements; }

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
  QList<AbstractType *> m_elements;
};

class ArrayType : public AbstractType
{
public:
  inline const QList<int>& dimensions () const
  { return m_dimensions; }

  inline void setDimensions (const QList<int>& dimensions)
  { m_dimensions = dimensions; }

  inline AbstractType *elementType () const
  { return m_elementType; }

  inline void setElementType(AbstractType * type)
  { m_elementType = type; }

  inline bool operator == (const ArrayType &other) const
  { return m_elementType == other.m_elementType && m_dimensions == other.m_dimensions; }

  inline bool operator != (const ArrayType &other) const
  { return m_elementType != other.m_elementType || m_dimensions != other.m_dimensions; }

private:
  QList<int> m_dimensions;
  AbstractType* m_elementType;
};

#endif // TYPESYSTEM_H
