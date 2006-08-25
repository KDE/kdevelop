/* This file is part of KDevelop
    Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2006 Adam Treat <treat@kde.org>

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

#ifndef CPPTYPES_H
#define CPPTYPES_H

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtCore/QPair>

#include "typesystem.h"
#include "typeinstance.h"

class DUContext;

namespace CppCodeModel {
  enum AccessPolicy
  {
    Public,
    Protected,
    Private
  };
}

class CppTypeInfo
{
public:
    bool isConstant() const { return m_constant; }
    void setConstant(bool is) { m_constant = is; }

    bool isVolatile() const { return m_volatile; }
    void setVolatile(bool is) { m_volatile = is; }

    bool operator==(const CppTypeInfo &other);
    bool operator!=(const CppTypeInfo &other) { return !(*this==other); }

private:
    uint m_constant : 1;
    uint m_volatile : 1;
};

class CppIntegralType : public IntegralType, public CppTypeInfo
{
};

class CppClassType;

struct CppBaseClassInstance
{
  CppClassType* baseClass;
  CppCodeModel::AccessPolicy access;
  bool virtualInheritance;
};

class CppClassType : public StructureType, public CppTypeInfo
{
public:
  CppClassType(DUContext* context);

  const QList<CppBaseClassInstance>& baseClasses() const;

  void addBaseClass(const CppBaseClassInstance& baseClass);
  void removeBaseClass(CppClassType* baseClass);

  const QList<CppClassType*>& subClasses() const;
  void addSubClass(CppClassType* subClass);
  void removeSubClass(CppClassType* subClass);

  enum ClassType
  {
    Class,
    Struct,
    Union
  };

  void setClassType(ClassType type);
  ClassType classType() const;

private:
  QList<CppBaseClassInstance> m_baseClasses;
  ClassType m_classType;
};

class CppClassMemberType : public CppTypeInfo
{
public:
  CppClassMemberType(CppClassType* owner);

  bool isStatic() const;
  void setStatic(bool isStatic);

  bool isAuto() const;
  void setAuto(bool isAuto);

  bool isFriend() const;
  void setFriend(bool isFriend);

  bool isRegister() const;
  void setRegister(bool isRegister);

  bool isExtern() const;
  void setExtern(bool isExtern);

  bool isMutable() const;
  void setMutable(bool isMutable);

  CppCodeModel::AccessPolicy accessPolicy() const;
  void setAccessPolicy(CppCodeModel::AccessPolicy accessPolicy);

private:
  uint m_isStatic: 1;
  uint m_isAuto: 1;
  uint m_isFriend: 1;
  uint m_isRegister: 1;
  uint m_isExtern: 1;
  uint m_isMutable: 1;
  CppCodeModel::AccessPolicy m_accessPolicy;
};

template <class T>
class CppSpecificClassMemberType : public T, public CppClassMemberType
{
public:
  CppSpecificClassMemberType(CppClassType* owner)
    : CppClassMemberType(owner)
  {
  }
};

class CppClassFunctionType : public FunctionType, public CppClassMemberType
{
public:
  CppClassFunctionType(CppClassType* owner);

  void addArgument(AbstractType* type, CppCodeModel::AccessPolicy policy);
  void removeArgument(AbstractType* type);

  enum QtFunctionType
  {
    Normal,
    Signal,
    Slot
  };

  QtFunctionType functionType() const;
  void setFunctionType(QtFunctionType functionType);

  bool isConstructor() const;
  bool isDestructor() const;

  bool isVirtual() const;
  void setVirtual(bool isVirtual);

  bool isInline() const;
  void setInline(bool isInline);

  bool isExplicit() const;
  void setExplicit(bool isExplicit);

  bool isAbstract() const;
  void setAbstract(bool isAbstract);

  //bool isSimilar(KDevCodeItem *other, bool strict = true) const;

private:
  bool m_constructor;
  bool m_destructor;
  QList<CppCodeModel::AccessPolicy> m_argumentAccessPolicies;
  QtFunctionType m_functionType;
  bool m_isVirtual: 1;
  bool m_isInline: 1;
  bool m_isAbstract: 1;
  bool m_isExplicit: 1;
};

// FIXME is IntegralType correct?
class CppTypeAliasType : public IntegralType, public CppTypeInfo
{
public:
  CppTypeAliasType(DUContext* context);

  AbstractType* type() const;
  void setType(AbstractType* type);

private:
  AbstractType* m_type;
};

class CppEnumeratorType;

// TODO is this the correct base type?
class CppEnumerationType : public StructureType
{
public:
  CppCodeModel::AccessPolicy accessPolicy() const;
  void setAccessPolicy(CppCodeModel::AccessPolicy accessPolicy);

  const QList<CppEnumeratorType*>& enumerators() const;
  void addEnumerator(CppEnumeratorType* item);
  void removeEnumerator(CppEnumeratorType* item);

private:
  CppCodeModel::AccessPolicy m_accessPolicy;
  QList<CppEnumeratorType*> m_enumerators;
};

class CppEnumeratorType : public IntegralType
{
public:
  CppEnumeratorType(CppEnumerationType* enumeration);

  QString value() const;
  void setValue(const QString &value);

private:
  QString m_value;
};

/*class CppTemplateParameterType : public
{
public:
  TypeInfo type() const;
  void setType(const TypeInfo &type);

  bool defaultValue() const;
  void setDefaultValue(bool defaultValue);

private:
  TypeInfo m_type;
  bool m_defaultValue;
};

class _TemplateModelItem: public _CodeModelItem
{
public:
  DECLARE_MODEL_NODE(Template)

  static TemplateModelItem create(CodeModel *model);

public:
  TemplateParameterList parameters() const;
  void addParameter(TemplateParameterModelItem item);
  void removeParameter(TemplateParameterModelItem item);

  CodeModelItem declaration() const;
  void setDeclaration(CodeModelItem declaration);

protected:
  _TemplateModelItem(CodeModel *model, int kind = __node_kind)
    : _CodeModelItem(model, kind) {}

private:
  TemplateParameterList m_parameters;
  CodeModelItem m_declaration;

private:
  _TemplateModelItem(const _TemplateModelItem &other);
  void operator = (const _TemplateModelItem &other);
};*/

/*template <class _Target, class _Source>
_Target model_static_cast(_Source item)
{
  typedef typename _Target::Type * _Target_pointer;

  _Target ptr = static_cast<_Target_pointer>(item.data());
  return ptr;
}

template <class _Target, class _Source>
_Target model_safe_cast(_Source item)
{
  typedef typename _Target::Type * _Target_pointer;
  typedef typename _Source::Type * _Source_pointer;

  _Source_pointer source = item.data();
  if (source && source->kind() == _Target_pointer(0)->__node_kind)
    {
      _Target ptr(static_cast<_Target_pointer>(source));
      return ptr;
    }

  return _Target();
}

template <typename _Target, typename _Source>
_Target model_dynamic_cast(_Source item)
{
  typedef typename _Target::Type * _Target_pointer;
  typedef typename _Source::Type * _Source_pointer;

  _Source_pointer source = item.data();
  if (source && (source->kind() == _Target_pointer(0)->__node_kind
         || (_Target_pointer(0)->__node_kind <= int(_CodeModelItem::KindMask)
             && ((source->kind() & _Target_pointer(0)->__node_kind)
                  == _Target_pointer(0)->__node_kind))))
    {
      _Target ptr(static_cast<_Target_pointer>(source));
      return ptr;
    }

  return _Target();
}*/

#endif // CPPTYPES_H

// kate: space-indent on; indent-width 2; replace-tabs on;
