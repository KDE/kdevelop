/* This file is part of KDevelop
    Copyright (C) 2006 Roberto Raggi <roberto@kdevelop.org>

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
  IntegralType (const QString *name):
    _M_name (name) {}

  inline const QString *name () const
  { return _M_name; }

  inline bool operator == (const IntegralType &__other) const
  { return _M_name == __other._M_name; }

  inline bool operator != (const IntegralType &__other) const
  { return _M_name != __other._M_name; }

protected:
  virtual void accept0 (TypeVisitor *v) const
  { v->visit (this); }

private:
  const QString *_M_name;
};

class PointerType: public AbstractType
{
public:
  PointerType (const AbstractType *baseType):
    _M_baseType (baseType) {}

  inline const AbstractType *baseType () const
  { return _M_baseType; }

  inline bool operator == (const PointerType &__other) const
  { return _M_baseType == __other._M_baseType; }

  inline bool operator != (const PointerType &__other) const
  { return _M_baseType != __other._M_baseType; }

protected:
  virtual void accept0 (TypeVisitor *v) const
  {
    if (v->visit (this))
      acceptType (_M_baseType, v);

    v->endVisit (this);
  }

private:
  const AbstractType *_M_baseType;
};

class ReferenceType: public AbstractType
{
public:
  ReferenceType (const AbstractType *baseType):
    _M_baseType (baseType) {}

  inline const AbstractType *baseType () const
  { return _M_baseType; }

  inline bool operator == (const ReferenceType &__other) const
  { return _M_baseType == __other._M_baseType; }

  inline bool operator != (const ReferenceType &__other) const
  { return _M_baseType != __other._M_baseType; }

protected:
  virtual void accept0 (TypeVisitor *v) const
  {
    if (v->visit (this))
      acceptType (_M_baseType, v);

    v->endVisit (this);
  }

private:
  const AbstractType *_M_baseType;
};

class FunctionType: public AbstractType
{
public:
  FunctionType (const AbstractType *returnType, const QVector<const AbstractType *> &arguments):
    _M_returnType (returnType),
    _M_arguments (arguments) {}

  inline const AbstractType *returnType () const
  { return _M_returnType; }

  inline const QVector<const AbstractType *>& arguments () const
  { return _M_arguments; }

  inline bool operator == (const FunctionType &__other) const
  { return _M_returnType == __other._M_returnType && _M_arguments == __other._M_arguments; }

  inline bool operator != (const FunctionType &__other) const
  { return _M_returnType != __other._M_returnType || _M_arguments != __other._M_arguments; }

protected:
  virtual void accept0 (TypeVisitor *v) const
  {
    if (v->visit (this))
      {
        acceptType (_M_returnType, v);

        for (int i = 0; i < _M_arguments.count (); ++i)
          acceptType (_M_arguments.at (i), v);
      }

    v->endVisit (this);
  }

private:
  const AbstractType *_M_returnType;
  QVector<const AbstractType *> _M_arguments;
};

class StructureType : public AbstractType
{
public:
  StructureType (const QVector<const AbstractType *> &elements):
    _M_elements (elements) {}

  inline const QVector<const AbstractType *>& elements () const
  { return _M_elements; }

  inline bool operator == (const StructureType &__other) const
  { return _M_elements == __other._M_elements; }

  inline bool operator != (const StructureType &__other) const
  { return _M_elements != __other._M_elements; }

private:
  QVector<const AbstractType *> _M_elements;
};

class ArrayType : public AbstractType
{
public:
  ArrayType (const QVector<int> &dimensions, const AbstractType* elementType):
    _M_dimensions (dimensions),
    _M_elementType (elementType) {}

  inline const QVector<int>& dimensions () const
  { return _M_dimensions; }

  inline const AbstractType *elementType () const
  { return _M_elementType; }

  inline bool operator == (const ArrayType &__other) const
  { return _M_elementType == __other._M_elementType && _M_dimensions == __other._M_dimensions; }

  inline bool operator != (const ArrayType &__other) const
  { return _M_elementType != __other._M_elementType || _M_dimensions != __other._M_dimensions; }

private:
  QVector<int> _M_dimensions;
  const AbstractType* _M_elementType;
};

/**
 * A class which creates types and holds a reference to those types.
 *
 * \todo there's something wrong with memory management here (not surprising given the magic in use)
 */
class TypeEnvironment
{
public:
  typedef QSet<QString> NameTable;
  typedef QSet<IntegralType> IntegralTypeTable;
  typedef QSet<PointerType> PointerTypeTable;
  typedef QSet<ReferenceType> ReferenceTypeTable;
  typedef QSet<FunctionType> FunctionTypeTable;

public:
  TypeEnvironment ();

  const QString *intern (const QString &name);

  const IntegralType *integralType (const QString *name);
  const PointerType *pointerType (const AbstractType *baseType);
  const ReferenceType *referenceType (const AbstractType *baseType);

  const FunctionType *functionType (const AbstractType *returnType, const QVector<const AbstractType *> &arguments);
  const FunctionType *functionType (const AbstractType *returnType);
  const FunctionType *functionType (const AbstractType *returnType, const AbstractType *arg_1);
  const FunctionType *functionType (const AbstractType *returnType, const AbstractType *arg_1,
                                    const AbstractType *arg_2);
  const FunctionType *functionType (const AbstractType *returnType, const AbstractType *arg_1,
                                    const AbstractType *arg_2, const AbstractType *arg_3);
  const FunctionType *functionType (const AbstractType *returnType, const AbstractType *arg_1,
                                    const AbstractType *arg_2, const AbstractType *arg_3,
                                    const AbstractType *arg_4);

private:
  NameTable _M_name_table;
  IntegralTypeTable _M_integral_type_table;
  PointerTypeTable _M_pointer_type_table;
  ReferenceTypeTable _M_reference_type_table;
  FunctionTypeTable _M_function_type_table;
};

uint qHash (const IntegralType &t);
uint qHash (const PointerType &t);
uint qHash (const ReferenceType &t);
uint qHash (const FunctionType &t);

#endif // TYPESYSTEM_H
