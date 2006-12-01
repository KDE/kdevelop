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

#include <kdevexport.h>
#include "typesystem.h"
#include "cppnamespace.h"

class DUContext;
class Declaration;
class ClassFunctionDeclaration;

class KDEVCPPLANGUAGE_EXPORT CppCVType
{
  friend class TypeRepository;
  friend class TypeBuilder;

public:
  CppCVType(Cpp::CVSpecs spec = Cpp::CVNone);

  inline bool isConstant() const { return m_constant; }

  inline bool isVolatile() const { return m_volatile; }

  QString cvString() const;
  QString cvMangled() const;

  //uint cvHash(uint input) const { return input; }

  Cpp::CVSpecs cv() const { return static_cast<Cpp::CVSpecs>((m_constant & Cpp::Const) | (m_volatile & Cpp::Volatile)); }

protected:
  inline void setCV(Cpp::CVSpecs spec) { m_constant = spec & Cpp::Const; m_volatile = spec & Cpp::Volatile; }
  inline void setConstant(bool is) { m_constant = is; }
  inline void setVolatile(bool is) { m_volatile = is; }

private:
  bool m_constant : 1;
  bool m_volatile : 1;
};

class CppIdentifiedType
{
public:
  CppIdentifiedType();

  QualifiedIdentifier identifier() const;

  Declaration* declaration() const;
  void setDeclaration(Declaration* declaration);

  QString idMangled() const;

private:
  Declaration* m_declaration;
};

class KDEVCPPLANGUAGE_EXPORT CppIntegralType : public IntegralType, public CppCVType
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
    ModifierLongLong  = 0x4,
    ModifierSigned    = 0x8,
    ModifierUnsigned  = 0x10
  };
  Q_DECLARE_FLAGS(TypeModifiers, TypeModifier)

  TypeModifiers typeModifiers() const;

  virtual QString toString() const;

  virtual QString mangled() const;

  //virtual uint hash() const;

protected:
  CppIntegralType(IntegralTypes type, CppIntegralType::TypeModifiers modifiers = ModifierNone);

private:
  IntegralTypes m_type;
  CppIntegralType::TypeModifiers m_modifiers;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(CppIntegralType::TypeModifiers)

class KDEVCPPLANGUAGE_EXPORT CppFunctionType : public FunctionType, public CppIdentifiedType, public CppCVType
{
public:
  typedef KSharedPtr<CppFunctionType> Ptr;

  ClassFunctionDeclaration* declaration() const;
  void setDeclaration(ClassFunctionDeclaration* declaration);

  virtual QString toString() const;

  //virtual uint hash() const;

  virtual QString mangled() const;
};

class KDEVCPPLANGUAGE_EXPORT CppPointerType : public PointerType, public CppCVType
{
public:
  typedef KSharedPtr<CppPointerType> Ptr;

  CppPointerType(Cpp::CVSpecs spec = Cpp::CVNone);

  virtual QString toString() const;

  virtual QString mangled() const;

  //virtual uint hash() const;
};

class KDEVCPPLANGUAGE_EXPORT CppReferenceType : public ReferenceType, public CppCVType
{
public:
  typedef KSharedPtr<CppReferenceType> Ptr;

  CppReferenceType(Cpp::CVSpecs spec = Cpp::CVNone);

  virtual QString toString() const;

  virtual QString mangled() const;

  //virtual uint hash() const;
};

class CppClassType;

class KDEVCPPLANGUAGE_EXPORT CppClassType : public StructureType, public CppIdentifiedType, public CppCVType
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

  /// C++ classes are closed types, once they are defined, they can't be changed.
  bool isClosed() const { return m_closed; }
  void close() { m_closed = true; }

  /// Error if the type is closed.
  virtual void addElement(AbstractType::Ptr element);

  //virtual uint hash() const;

  virtual QString toString() const;

  virtual QString mangled() const;

private:
  QList<BaseClassInstance> m_baseClasses;
  ClassType m_classType;
  bool m_closed;
};

/*class CppArrayType : public ArrayType, public CppCVType
{
public:
  typedef KSharedPtr<CppArrayType> Ptr;

  CppArrayType(Cpp::CVSpecs spec = Cpp::CVNone);

  virtual QString toString() const;
};*/

class CppTypeAliasType : public AbstractType, public CppIdentifiedType, public CppCVType
{
public:
  typedef KSharedPtr<CppTypeAliasType> Ptr;

  CppTypeAliasType();

  AbstractType::Ptr type() const;
  void setType(AbstractType::Ptr type);

  //virtual uint hash() const;

  virtual QString toString() const;

  virtual QString mangled() const;

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
};*/

class KDEVCPPLANGUAGE_EXPORT CppEnumerationType : public CppIntegralType, public CppIdentifiedType
{
public:
  typedef KSharedPtr<CppEnumerationType> Ptr;

  CppEnumerationType(Cpp::CVSpecs spec = Cpp::CVNone);
  //virtual uint hash() const;

  virtual QString mangled() const;
};

class CppArrayType : public ArrayType
{
public:
  typedef KSharedPtr<CppArrayType> Ptr;

  virtual QString mangled() const;
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
};*/

class CppTemplateType : public AbstractType, public CppIdentifiedType
{
public:
  typedef KSharedPtr<CppTemplateType> Ptr;

  const QList<AbstractType::Ptr>& parameters() const;
  void addParameter(AbstractType::Ptr parameter);
  void removeParameter(AbstractType::Ptr parameter);

private:
  QList<AbstractType::Ptr> m_parameters;
};

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
