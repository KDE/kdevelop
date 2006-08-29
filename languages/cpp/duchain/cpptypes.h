/* This file is part of KDevelop
    Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2006 Adam Treat <treat@kde.org>
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

#ifndef CPPTYPES_H
#define CPPTYPES_H

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtCore/QPair>

#include "typesystem.h"
#include "cppnamespace.h"

class DUContext;

class CppCVType
{
  friend class TypeRepository;
  friend class TypeBuilder;

public:
  CppCVType(Cpp::CVSpecs spec = Cpp::CVNone);

  inline bool isConstant() const { return m_constant; }

  inline bool isVolatile() const { return m_volatile; }

  QString cvString() const;

  uint cvHash(uint input) const { return input; }

private:
  inline void setCV(Cpp::CVSpecs spec) { m_constant = spec & Cpp::Const; m_volatile = spec & Cpp::Volatile; }
  inline void setConstant(bool is) { m_constant = is; }
  inline void setVolatile(bool is) { m_volatile = is; }

  bool m_constant : 1;
  bool m_volatile : 1;
};

class CppIntegralType : public IntegralType, public CppCVType
{
  friend class TypeRepository;

public:
  typedef KSharedPtr<CppIntegralType> Ptr;

  enum IntegralTypes {
    TypeNone,
    TypeChar,
    TypeWchar_t,  // C++ only
    TypeBool, // C++ only
    TypeInt,
    TypeFloat,
    TypeDouble,
    TypeVoid
  };

  IntegralTypes integralType() const;

  enum TypeModifier {
    ModifierNone      = 0x0,
    ModifierShort     = 0x1,
    ModifierLong      = 0x2,
    ModifierSigned    = 0x4,
    ModifierUnsigned  = 0x8
  };
  Q_DECLARE_FLAGS(TypeModifiers, TypeModifier)

  TypeModifiers typeModifiers() const;

  virtual QString toString() const;

  virtual uint hash() const;

private:
  CppIntegralType(IntegralTypes type, CppIntegralType::TypeModifiers modifiers = ModifierNone);

  IntegralTypes m_type;
  CppIntegralType::TypeModifiers m_modifiers;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(CppIntegralType::TypeModifiers)

class CppFunctionType : public FunctionType, public CppCVType
{
public:
  typedef KSharedPtr<CppFunctionType> Ptr;

  virtual QString toString() const;

  virtual uint hash() const;
};

class CppPointerType : public PointerType, public CppCVType
{
public:
  typedef KSharedPtr<CppPointerType> Ptr;

  CppPointerType(Cpp::CVSpecs spec = Cpp::CVNone);

  virtual QString toString() const;

  virtual uint hash() const;
};

class CppReferenceType : public ReferenceType, public CppCVType
{
public:
  typedef KSharedPtr<CppReferenceType> Ptr;

  CppReferenceType(Cpp::CVSpecs spec = Cpp::CVNone);

  virtual QString toString() const;

  virtual uint hash() const;
};

class CppClassType;

class CppClassType : public StructureType, public CppCVType
{
public:
  typedef KSharedPtr<CppClassType> Ptr;

  CppClassType(Cpp::CVSpecs spec = Cpp::CVNone);

  struct BaseClassInstance
  {
    CppClassType::Ptr baseClass;
    Cpp::AccessPolicy access;
    bool virtualInheritance;
  };

  const QList<BaseClassInstance>& baseClasses() const;
  void addBaseClass(const BaseClassInstance& baseClass);
  void removeBaseClass(CppClassType::Ptr baseClass);

  const QList<CppClassType::Ptr>& subClasses() const;
  void addSubClass(CppClassType::Ptr subClass);
  void removeSubClass(CppClassType::Ptr subClass);

  enum ClassType
  {
    Class,
    Struct,
    Union
  };

  void setClassType(ClassType type);
  ClassType classType() const;

  virtual uint hash() const;

private:
  QList<BaseClassInstance> m_baseClasses;
  ClassType m_classType;
};

/*class CppArrayType : public ArrayType, public CppCVType
{
public:
  typedef KSharedPtr<CppArrayType> Ptr;

  CppArrayType(Cpp::CVSpecs spec = Cpp::CVNone);

  virtual QString toString() const;
};*/

class CppTypeAliasType : public AbstractType, public CppCVType
{
public:
  typedef KSharedPtr<CppTypeAliasType> Ptr;

  CppTypeAliasType();

  AbstractType::Ptr type() const;
  void setType(AbstractType::Ptr type);

  virtual uint hash() const;

  virtual QString toString() const;

protected:
  virtual void accept0 (TypeVisitor *v) const
  {
    //if (v->visit (this))
      acceptType (m_type, v);

    //v->endVisit (this);
  }

private:
  AbstractType::Ptr m_type;
};

/*class CppEnumeratorType : public IntegralType
{
public:
  typedef KSharedPtr<CppEnumeratorType> Ptr;

  CppEnumeratorType();

  QString value() const;
  void setValue(const QString &value);

  virtual uint hash() const;

private:
  QString m_value;
};

// TODO is this the correct base type?
class CppEnumerationType : public StructureType
{
public:
  typedef KSharedPtr<CppEnumerationType> Ptr;

  const QList<CppEnumeratorType::Ptr>& enumerators() const;
  void addEnumerator(CppEnumeratorType::Ptr item);
  void removeEnumerator(CppEnumeratorType::Ptr item);

  virtual uint hash() const;

private:
  QList<CppEnumeratorType::Ptr> m_enumerators;
};*/

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
