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

class AbstractType;
class IntegralType;
class PointerType;

class TypeVisitor
{
public:
  virtual ~TypeVisitor ();

  virtual bool preVisit (const AbstractType *) { return true; }
  virtual void postVisit (const AbstractType *) {}

  virtual void visit (const IntegralType *) {}

  virtual bool visit (const PointerType *) { return true; }
  virtual void endVisit (const PointerType *) {}
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
      acceptType (baseType (), v);

    v->endVisit (this);
  }

private:
  const AbstractType *_M_baseType;
};

class TypeEnvironment
{
public:
  typedef QSet<QString> NameTable;
  typedef QSet<IntegralType> IntegralTypeTable;
  typedef QSet<PointerType> PointerTypeTable;

public:
  TypeEnvironment ();

  const QString *intern (const QString &name);

  const IntegralType *integralType (const QString *name);
  const PointerType *pointerType (const AbstractType *baseType);

private:
  NameTable _M_name_table;
  IntegralTypeTable _M_integral_type_table;
  PointerTypeTable _M_pointer_type_table;
};

uint qHash (const IntegralType &t);
uint qHash (const PointerType &t);

#endif // TYPESYSTEM_H
