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

#ifndef CPPTYPES_H
#define CPPTYPES_H

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtCore/QPair>
#include <QtCore/QVarLengthArray>

#include <duchain/identifier.h>
#include <duchain/typesystemdata.h>
#include <typesystem.h>
#include <declaration.h>
#include <identifiedtype.h>
#include "cppduchainexport.h"

namespace KDevelop
{
class DUContext;
class Declaration;
}

template<typename T>
T constant_value(const qint64* realval)
{
  T value;
  memcpy(&value, realval, sizeof(T));
  return value;
}

class TemplateParameterDeclaration;

class CppCVTypeData {
  public:
  CppCVTypeData() : m_constant(false), m_volatile(false) {
  }
  bool m_constant : 1;
  bool m_volatile : 1;
};

class KDEVCPPDUCHAIN_EXPORT CppCVType
{
  friend class TypeRepository;
  friend class TypeBuilder;

public:
  CppCVType() {
  }
  
  virtual ~CppCVType() {
  }

  inline bool isConstant() const { return cvData()->m_constant; }

  inline bool isVolatile() const { return cvData()->m_volatile; }

  void clear();
  
  QString cvString() const;
  QString cvMangled() const;

  uint cvHash(uint input) const { return input + (cvData()->m_constant ? 7 : 0) + (cvData()->m_volatile ? 3 : 0); }

  KDevelop::Declaration::CVSpecs cv() const { return static_cast<KDevelop::Declaration::CVSpecs>((cvData()->m_constant & KDevelop::Declaration::Const) | (cvData()->m_volatile & KDevelop::Declaration::Volatile)); }

  bool equals(const CppCVType* rhs) const;
  
  virtual CppCVTypeData* cvData() = 0;
  virtual const CppCVTypeData* cvData() const = 0;
  
  inline void setCV(KDevelop::Declaration::CVSpecs spec) { cvData()->m_constant = spec & KDevelop::Declaration::Const; cvData()->m_volatile = spec & KDevelop::Declaration::Volatile; }
  
protected:
  inline void setConstant(bool is) { cvData()->m_constant = is; }
  inline void setVolatile(bool is) { cvData()->m_volatile = is; }
};

template<class Parent>
class KDEVPLATFORMLANGUAGE_EXPORT MergeCppCVType : public Parent, public CppCVType {
  public:
    class Data : public Parent::Data, public CppCVTypeData {
    };
    MergeCppCVType(Data& data) : Parent(data) {
    }
    virtual CppCVTypeData* cvData() {
      return static_cast<Data*>(this->d_func_dynamic());
    }
    
    virtual const CppCVTypeData* cvData() const {
      return static_cast<const Data*>(this->d_func());
    }
};


enum IntegralTypes {
  TypeVoid,
  TypeNone,
  TypeChar,
  TypeBool, // C++ only
  TypeWchar_t,  // C++ only
  TypeInt,
  TypeFloat,
  TypeDouble
};

enum TypeModifiers {
  ModifierNone      = 0x0,
  ModifierShort     = 0x1,
  ModifierLong      = 0x2,
  ModifierLongLong  = 0x4,
  ModifierSigned    = 0x8,
  ModifierUnsigned  = 0x10
};

struct CppIntegralTypeData : public MergeCppCVType< KDevelop::IntegralType >::Data {
  IntegralTypes m_type;
  TypeModifiers m_modifiers;
  CppIntegralTypeData() : m_type(TypeInt), m_modifiers(ModifierNone) {
  }
};

class KDEVCPPDUCHAIN_EXPORT CppIntegralType : public MergeCppCVType< KDevelop::IntegralType >
{
  friend class TypeRepository;

public:
  CppIntegralType() : MergeCppCVType< KDevelop::IntegralType >(createData<CppIntegralTypeData>()) {
    d_func_dynamic()->setTypeClassId<CppIntegralType>();
  }
  CppIntegralType(const CppIntegralType& rhs) : MergeCppCVType< KDevelop::IntegralType >(copyData<CppIntegralTypeData>(*rhs.d_func())) {
  }
  CppIntegralType(CppIntegralTypeData& data) : MergeCppCVType< KDevelop::IntegralType >(data) {
  }
  CppIntegralType(IntegralTypes type, TypeModifiers modifiers = ModifierNone);
  
  typedef TypePtr<CppIntegralType> Ptr;

  IntegralTypes integralType() const;
  
  void setIntegralType(IntegralTypes t) {
    d_func_dynamic()->m_type = t;
  }
  
  void setTypeModifiers(TypeModifiers mod) {
    d_func_dynamic()->m_modifiers = mod;
  }

  TypeModifiers typeModifiers() const;

  virtual QString toString() const;

//   virtual QString mangled() const;

  virtual uint hash() const;

  virtual KDevelop::AbstractType* clone() const;

  virtual bool equals(const KDevelop::AbstractType* rhs) const;

  //A simple hack that tries taking the modifiers and the type itself into account
  bool moreExpressiveThan(CppIntegralType* rhs) const;
  
  typedef CppIntegralTypeData Data;
  
  enum {
    Identity = 13
  };
  
protected:
  TYPE_DECLARE_DATA(CppIntegralType);
};

struct CppConstantIntegralTypeData : public CppIntegralTypeData {
  qint64 m_value;
  CppConstantIntegralTypeData() : m_value(0) {
  }
};

/**
 * An integral type that additionally has a constant value
 * */
class KDEVCPPDUCHAIN_EXPORT CppConstantIntegralType : public CppIntegralType
{
  friend class TypeRepository;

public:
  CppConstantIntegralType(const CppConstantIntegralType& rhs) : CppIntegralType(copyData<CppConstantIntegralTypeData>(*rhs.d_func())) {
  }
  CppConstantIntegralType(CppConstantIntegralTypeData& data) : CppIntegralType(data) {
  }
  CppConstantIntegralType() : CppIntegralType(createData<CppConstantIntegralTypeData>()) {
    d_func_dynamic()->setTypeClassId<CppConstantIntegralType>();
  }
  CppConstantIntegralType(IntegralTypes type, TypeModifiers modifiers = ModifierNone);
  
  typedef TypePtr<CppConstantIntegralType> Ptr;

  /**The types and modifiers are not changed!
   * The values are casted internally to the local representation, so you can lose precision.
   * */
  template<class ValueType>
  void setValue(ValueType value) {
    if(typeModifiers() & ModifierUnsigned)
      setValueInternal<quint64>(value);
    else if(integralType() == TypeFloat)
      setValueInternal<float>(value);
    else if(integralType() == TypeDouble)
      setValueInternal<double>(value);
    else
      setValueInternal<qint64>(value);
  }

  /**
   * For booleans, the value is 1 for true, and 0 for false.
   * All signed values should be retrieved and set through value(),
   * 
   * */
  template<class ValueType>
  ValueType value() const {
    if(typeModifiers() & ModifierUnsigned) {
      return constant_value<quint64>(&d_func()->m_value);
    } else if(integralType() == TypeFloat) {
      return constant_value<float>(&d_func()->m_value);
    } else if(integralType() == TypeDouble) {
      return constant_value<double>(&d_func()->m_value);
    } else {
      return constant_value<qint64>(&d_func()->m_value);
    }
  }
  
  qint64 plainValue() const {
    return d_func()->m_value;
  }

  virtual QString toString() const;

  virtual KDevelop::AbstractType* clone() const;

  virtual uint hash() const;
  
  virtual bool equals(const KDevelop::AbstractType* rhs) const;
  
  typedef CppConstantIntegralTypeData Data;
  
  enum {
    Identity = 14
  };
  
protected:
  TYPE_DECLARE_DATA(CppConstantIntegralType);
private:
  //Sets the value without casting
  template<class ValueType>
  void setValueInternal(ValueType value);
};

TypeModifiers& operator|=(TypeModifiers& lhs, const TypeModifiers& rhs);

TypeModifiers operator|(const TypeModifiers& lhs, const TypeModifiers& rhs);
//Q_DECLARE_OPERATORS_FOR_FLAGS(TypeModifiers);

typedef MergeCppCVType< KDevelop::FunctionType > CppFunctionTypeBase;

struct CppFunctionTypeData : public CppFunctionTypeBase::Data {
};
    
class KDEVCPPDUCHAIN_EXPORT CppFunctionType : public CppFunctionTypeBase
{
public:
  CppFunctionType(const CppFunctionType& rhs) : CppFunctionTypeBase(copyData<CppFunctionTypeData>(*rhs.d_func())) {
  }
  
  CppFunctionType(CppFunctionTypeData& data) : CppFunctionTypeBase(data) {
  }
  
  CppFunctionType() : CppFunctionTypeBase(createData<CppFunctionTypeData>()) {
    d_func_dynamic()->setTypeClassId<CppFunctionType>();
  }
  
  typedef TypePtr<CppFunctionType> Ptr;

  ///Declarations of this class(@see KDevelop::IdentifiedType::declaration()) are guaranteed to be based on AbstractFunctionDeclaration
  
  virtual QString toString() const;

  virtual uint hash() const;

//   virtual QString mangled() const;

  virtual KDevelop::AbstractType* clone() const;

  virtual bool equals(const KDevelop::AbstractType* rhs) const;
  
  enum {
    Identity = 15
  };
  
  typedef CppFunctionTypeData Data;
  
  protected:
    TYPE_DECLARE_DATA(CppFunctionType);
};

typedef MergeCppCVType<KDevelop::PointerType> CppPointerTypeBase;

struct CppPointerTypeData : public CppPointerTypeBase::Data {
};

class KDEVCPPDUCHAIN_EXPORT CppPointerType : public CppPointerTypeBase
{
public:
  CppPointerType(const CppPointerType& rhs) : CppPointerTypeBase(copyData<CppPointerTypeData>(*rhs.d_func())) {
  }
  
  CppPointerType(CppPointerTypeData& data) : CppPointerTypeBase(data) {
  }
  
  CppPointerType() : CppPointerTypeBase(createData<CppPointerTypeData>()) {
    d_func_dynamic()->setTypeClassId<CppPointerType>();
  }
  
  typedef TypePtr<CppPointerType> Ptr;

  CppPointerType(KDevelop::Declaration::CVSpecs spec = KDevelop::Declaration::CVNone);

  virtual QString toString() const;

//   virtual QString mangled() const;

  virtual KDevelop::AbstractType* clone() const;
  
  virtual bool equals(const KDevelop::AbstractType* rhs) const;

  virtual uint hash() const;
  
  enum {
    Identity = 16
  };
  
  typedef CppPointerTypeData Data;
  
protected:
  TYPE_DECLARE_DATA(CppPointerType);
};

typedef MergeCppCVType<KDevelop::ReferenceType> CppReferenceTypeBase;

struct CppReferenceTypeData : public CppReferenceTypeBase::Data {
};

class KDEVCPPDUCHAIN_EXPORT CppReferenceType : public CppReferenceTypeBase
{
public:
  CppReferenceType(const CppReferenceType& rhs) : CppReferenceTypeBase(copyData<CppReferenceTypeData>(*rhs.d_func())) {
  }
  
  CppReferenceType(CppReferenceTypeData& data) : CppReferenceTypeBase(data) {
  }
  
  CppReferenceType() : CppReferenceTypeBase(createData<CppReferenceTypeData>()) {
    d_func_dynamic()->setTypeClassId<CppReferenceType>();
  }
  
  typedef TypePtr<CppReferenceType> Ptr;

  CppReferenceType(KDevelop::Declaration::CVSpecs spec = KDevelop::Declaration::CVNone);

  virtual QString toString() const;

//   virtual QString mangled() const;

  virtual KDevelop::AbstractType* clone() const;
  
  virtual bool equals(const KDevelop::AbstractType* rhs) const;

  virtual uint hash() const;
  
  enum {
    Identity = 17
  };
  
  typedef CppReferenceTypeData Data;
  
  protected:
  TYPE_DECLARE_DATA(CppReferenceType);
};

typedef MergeCppCVType< KDevelop::MergeIdentifiedType<KDevelop::StructureType> > CppClassTypeBase;

enum ClassType
{
  Class,
  Struct,
  Union
};

struct KDEVCPPDUCHAIN_EXPORT CppClassTypeData : public CppClassTypeBase::Data {
  ClassType m_classType;
  bool m_closed;
  
  CppClassTypeData() {
    m_classType = Class;
    m_closed = false;
  }
  
  CppClassTypeData(const CppClassTypeData& rhs) :CppClassTypeBase::Data(rhs), m_classType(rhs.m_classType), m_closed(rhs.m_closed)  {
  }
  
  ~CppClassTypeData() {
  }
  
  private:
    CppClassTypeData& operator=(const CppClassTypeData&) {
      return *this;
    }
};

class KDEVCPPDUCHAIN_EXPORT CppClassType : public CppClassTypeBase
{
public:
  CppClassType(const CppClassType& rhs) : CppClassTypeBase(copyData<CppClassTypeData>(*rhs.d_func())) {
  }
  
  CppClassType(CppClassTypeData& data) : CppClassTypeBase(data) {
  }
  
  typedef TypePtr<CppClassType> Ptr;

  CppClassType(KDevelop::Declaration::CVSpecs spec = KDevelop::Declaration::CVNone);

  void setClassType(ClassType type);
  ClassType classType() const;

  /// C++ classes are closed types, once they are defined, they can't be changed.
  bool isClosed() const { return d_func()->m_closed; }
  void close() { d_func_dynamic()->m_closed = true; }

  ///After clearing, a class-type is open again.
  void clear(); 

  virtual uint hash() const;

  virtual QString toString() const;

  virtual KDevelop::AbstractType* clone() const;

  virtual bool equals(const KDevelop::AbstractType* rhs) const;
  
  virtual void accept0 (KDevelop::TypeVisitor *v) const;

  virtual void exchangeTypes(KDevelop::TypeExchanger*);
  
  enum {
    Identity = 18
  };
  
  typedef CppClassTypeData Data;
  
protected:
  TYPE_DECLARE_DATA(CppClassType);
};

/*class CppArrayType : public ArrayType, public CppCVType
{
public:
  typedef TypePtr<CppArrayType> Ptr;

  CppArrayType(Declaration::CVSpecs spec = Declaration::CVNone);

  virtual QString toString() const;
};*/

typedef MergeCppCVType< KDevelop::MergeIdentifiedType<KDevelop::AbstractType> > CppTypeAliasTypeBase;

struct CppTypeAliasTypeData : public CppTypeAliasTypeBase::Data {
  KDevelop::IndexedType m_type;
};

class KDEVCPPDUCHAIN_EXPORT CppTypeAliasType : public CppTypeAliasTypeBase
{
public:
  typedef TypePtr<CppTypeAliasType> Ptr;

  CppTypeAliasType(const CppTypeAliasType& rhs) : CppTypeAliasTypeBase(copyData<CppTypeAliasTypeData>(*rhs.d_func())) {
  }
  
  CppTypeAliasType(CppTypeAliasTypeData& data) : CppTypeAliasTypeBase(data) {
  }
  
  CppTypeAliasType() : CppTypeAliasTypeBase(createData<CppTypeAliasTypeData>()) {
    d_func_dynamic()->setTypeClassId<CppTypeAliasType>();
  }

  KDevelop::AbstractType::Ptr type() const;
  void setType(KDevelop::AbstractType::Ptr type);

  virtual uint hash() const;

  virtual QString toString() const;

//   virtual QString mangled() const;

  virtual KDevelop::AbstractType* clone() const;
  
  virtual bool equals(const KDevelop::AbstractType* rhs) const;
  
  enum {
    Identity = 19
  };
  
  typedef CppTypeAliasTypeData Data;
  
protected:
  TYPE_DECLARE_DATA(CppTypeAliasType);
    
  virtual void accept0 (KDevelop::TypeVisitor *v) const
  {
    if (v->visit (this))
      acceptType (d_func()->m_type.type(), v);

    //v->endVisit (this);
  }
};

typedef KDevelop::MergeIdentifiedType<CppConstantIntegralType> CppEnumeratorTypeBase;

typedef CppEnumeratorTypeBase::Data CppEnumeratorTypeData;

//The same as CppEnumerationType, with the difference that here the value is also known
class KDEVCPPDUCHAIN_EXPORT CppEnumeratorType : public CppEnumeratorTypeBase
{
public:
  CppEnumeratorType(const CppEnumeratorType& rhs) : CppEnumeratorTypeBase(copyData<CppEnumeratorTypeData>(*rhs.d_func())) {
  }
  
  CppEnumeratorType(CppEnumeratorTypeData& data) : CppEnumeratorTypeBase(data) {
  }
  
  CppEnumeratorType() : CppEnumeratorTypeBase(createData<CppEnumeratorTypeData>()) {
    d_func_dynamic()->setTypeClassId<CppEnumeratorType>();
    CppConstantIntegralType::setIntegralType(TypeInt);
    setCV(KDevelop::Declaration::Const);
  }
  
  typedef TypePtr<CppEnumeratorType> Ptr;

  virtual QString toString() const;

  virtual bool equals(const KDevelop::AbstractType* rhs) const;

  virtual KDevelop::AbstractType* clone() const;

  virtual uint hash() const;
  
  enum {
    Identity = 20
  };
  
  typedef CppEnumeratorTypeData Data;
  
protected:
  TYPE_DECLARE_DATA(CppEnumeratorType);
};

typedef KDevelop::MergeIdentifiedType<CppIntegralType> CppEnumerationTypeBase;

typedef CppEnumerationTypeBase::Data CppEnumerationTypeData;

class KDEVCPPDUCHAIN_EXPORT CppEnumerationType : public CppEnumerationTypeBase
{
public:
  CppEnumerationType(const CppEnumerationType& rhs) : CppEnumerationTypeBase(copyData<CppEnumerationTypeData>(*rhs.d_func())) {
  }
  
  CppEnumerationType(CppEnumerationTypeData& data) : CppEnumerationTypeBase(data) {
  }
  
  typedef TypePtr<CppEnumerationType> Ptr;

  CppEnumerationType(KDevelop::Declaration::CVSpecs spec = KDevelop::Declaration::CVNone);

  virtual uint hash() const;

  virtual KDevelop::AbstractType* clone() const;
  
  virtual bool equals(const KDevelop::AbstractType* rhs) const;

//   virtual QString mangled() const;
  
  virtual QString toString() const;
  
  enum {
    Identity = 21
  };
  
  typedef CppEnumerationTypeData Data;
  
protected:
  TYPE_DECLARE_DATA(CppEnumerationType);
};

// class KDEVCPPDUCHAIN_EXPORT CppArrayType : public KDevelop::ArrayType
// {
// public:
//   typedef TypePtr<CppArrayType> Ptr;
// 
// //   virtual QString mangled() const;
// 
//   virtual KDevelop::AbstractType* clone() const;
//   
//   virtual bool equals(const KDevelop::AbstractType* rhs) const;
//   
//   enum {
//     Identity = 22
//   };
// };

/**
 * This class represents a template-parameter on the type-level(it is strictly attached to a template-declaration)
 * This is only attached to unset template-parameters. Once the template-parameters are set, the TemplateDeclarations
 * will return the real set types as abstractType().
 * This means that when you encounter this type, it means that the template-parameter is not set.
 * */

typedef KDevelop::MergeIdentifiedType<KDevelop::AbstractType> CppTemplateParameterTypeBase;

typedef CppTemplateParameterTypeBase::Data CppTemplateParameterTypeData;

class KDEVCPPDUCHAIN_EXPORT CppTemplateParameterType : public CppTemplateParameterTypeBase
{
public:
  CppTemplateParameterType(const CppTemplateParameterType& rhs) : CppTemplateParameterTypeBase(copyData<CppTemplateParameterTypeData>(*rhs.d_func())) {
  }
  
  CppTemplateParameterType(CppTemplateParameterTypeData& data) : CppTemplateParameterTypeBase(data) {
  }
  
  CppTemplateParameterType() : CppTemplateParameterTypeBase(createData<CppTemplateParameterTypeData>()) {
    d_func_dynamic()->setTypeClassId<CppTemplateParameterType>();
  }
  
  typedef TypePtr<CppTemplateParameterType> Ptr;

  TemplateParameterDeclaration* declaration(const KDevelop::TopDUContext* top) const;

  virtual QString toString() const;
//   virtual QString mangled() const;

  virtual KDevelop::AbstractType* clone() const;
  
  virtual bool equals(const KDevelop::AbstractType* rhs) const;

  virtual uint hash() const;
  
  enum {
    Identity = 23
  };
  
  typedef CppTemplateParameterTypeData Data;
  
  protected:
  virtual void accept0 (KDevelop::TypeVisitor *v) const;
  TYPE_DECLARE_DATA(CppTemplateParameterType);
};

namespace KDevelop {

template<>
inline CppReferenceType* fastCast<CppReferenceType*>(AbstractType* from) {
  if(!from || from->whichType() != KDevelop::AbstractType::TypeReference)
    return 0;
  else
    return dynamic_cast<CppReferenceType*>(static_cast<ReferenceType*>(from));
}

template<>
inline CppPointerType* fastCast<CppPointerType*>(AbstractType* from) {
  if(!from || from->whichType() != KDevelop::AbstractType::TypePointer)
    return 0;
  else
    return dynamic_cast<CppPointerType*>(static_cast<PointerType*>(from));
}

template<>
inline CppIntegralType* fastCast<CppIntegralType*>(AbstractType* from) {
  if(!from || from->whichType() != KDevelop::AbstractType::TypeIntegral)
    return 0;
  else
    return dynamic_cast<CppIntegralType*>(static_cast<IntegralType*>(from));
}

template<>
inline CppConstantIntegralType* fastCast<CppConstantIntegralType*>(AbstractType* from) {
  if(!from || from->whichType() != KDevelop::AbstractType::TypeIntegral)
    return 0;
  else
    return dynamic_cast<CppConstantIntegralType*>(static_cast<IntegralType*>(from));
}

template<>
inline CppFunctionType* fastCast<CppFunctionType*>(AbstractType* from) {
  if(!from || from->whichType() != KDevelop::AbstractType::TypeFunction)
    return 0;
  else
    return dynamic_cast<CppFunctionType*>(static_cast<FunctionType*>(from));
}

template<>
inline CppClassType* fastCast<CppClassType*>(AbstractType* from) {
  if(!from || from->whichType() != KDevelop::AbstractType::TypeStructure)
    return 0;
  else
    return dynamic_cast<CppClassType*>(static_cast<StructureType*>(from));
}

// template<>
// inline CppArrayType* fastCast<CppArrayType*>(AbstractType* from) {
//   if(!from || from->whichType() != KDevelop::AbstractType::TypeArray)
//     return 0;
//   else
//     return dynamic_cast<CppArrayType*>(static_cast<ArrayType*>(from));
// }

template<>
inline CppTemplateParameterType* fastCast<CppTemplateParameterType*>(AbstractType* from) {
  if(!from || from->whichType() != KDevelop::AbstractType::TypeAbstract)
    return 0;
  else
    return dynamic_cast<CppTemplateParameterType*>(static_cast<CppTemplateParameterType*>(from));
}

}


#endif // CPPTYPES_H

