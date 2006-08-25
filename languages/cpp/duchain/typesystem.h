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
#include <QVector>

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
  IntegralType(const Identifier& identifier);

  inline const Identifier& identifier() const
  { return m_identifier; }

  inline bool operator == (const IntegralType &other) const
  { return m_identifier == other.m_identifier; }

  inline bool operator != (const IntegralType &other) const
  { return m_identifier != other.m_identifier; }

protected:
  virtual void accept0 (TypeVisitor *v) const
  { v->visit (this); }

private:
  Identifier m_identifier;
};

class PointerType: public AbstractType
{
public:
  PointerType (const AbstractType *baseType):
    m_baseType (baseType) {}

  inline const AbstractType *baseType () const
  { return m_baseType; }

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
  const AbstractType *m_baseType;
};

class ReferenceType: public AbstractType
{
public:
  ReferenceType (const AbstractType *baseType):
    m_baseType (baseType) {}

  inline const AbstractType *baseType () const
  { return m_baseType; }

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
  const AbstractType *m_baseType;
};

class FunctionType: public AbstractType
{
public:
  FunctionType (const AbstractType *returnType, const QVector<const AbstractType *> &arguments):
    m_returnType (returnType),
    m_arguments (arguments) {}

  inline const AbstractType *returnType () const
  { return m_returnType; }

  inline const QVector<const AbstractType *>& arguments () const
  { return m_arguments; }

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
  const AbstractType *m_returnType;
  QVector<const AbstractType *> m_arguments;
};

class StructureType : public AbstractType
{
public:
  StructureType (const QVector<const AbstractType *> &elements):
    m_elements (elements) {}

  inline const QVector<const AbstractType *>& elements () const
  { return m_elements; }

  inline bool operator == (const StructureType &other) const
  { return m_elements == other.m_elements; }

  inline bool operator != (const StructureType &other) const
  { return m_elements != other.m_elements; }

private:
  QVector<const AbstractType *> m_elements;
};

class ArrayType : public AbstractType
{
public:
  ArrayType (const QVector<int> &dimensions, const AbstractType* elementType):
    m_dimensions (dimensions),
    m_elementType (elementType) {}

  inline const QVector<int>& dimensions () const
  { return m_dimensions; }

  inline const AbstractType *elementType () const
  { return m_elementType; }

  inline bool operator == (const ArrayType &other) const
  { return m_elementType == other.m_elementType && m_dimensions == other.m_dimensions; }

  inline bool operator != (const ArrayType &other) const
  { return m_elementType != other.m_elementType || m_dimensions != other.m_dimensions; }

private:
  QVector<int> m_dimensions;
  const AbstractType* m_elementType;
};

uint qHash (const IntegralType &t);
uint qHash (const PointerType &t);
uint qHash (const ReferenceType &t);
uint qHash (const FunctionType &t);

#endif // TYPESYSTEM_H
