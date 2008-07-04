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

#include "cpptypes.h"

#include <classfunctiondeclaration.h>
#include <abstractfunctiondeclaration.h>
#include <indexedstring.h>
#include <typeregister.h>
#include "templateparameterdeclaration.h"
#include <ducontext.h> //Only for FOREACH_ARRAY

using namespace KDevelop;

//Because all these classes have no d-pointers, shallow copies are perfectly fine

REGISTER_TYPE(CppFunctionType);
REGISTER_TYPE(CppConstantIntegralType);
REGISTER_TYPE(CppPointerType);
REGISTER_TYPE(CppReferenceType);
REGISTER_TYPE(CppClassType);
REGISTER_TYPE(CppTypeAliasType);
REGISTER_TYPE(CppEnumeratorType);
REGISTER_TYPE(CppEnumerationType);
// REGISTER_TYPE(CppArrayType);
REGISTER_TYPE(CppIntegralType);
REGISTER_TYPE(CppTemplateParameterType);

AbstractType* CppFunctionType::clone() const {
  return new CppFunctionType(*this);
}

AbstractType* CppConstantIntegralType::clone() const {
  return new CppConstantIntegralType(*this);
}

AbstractType* CppPointerType::clone() const {
  return new CppPointerType(*this);
}

AbstractType* CppReferenceType::clone() const {
  return new CppReferenceType(*this);
}

AbstractType* CppClassType::clone() const {
  return new CppClassType(*this);
}

AbstractType* CppTypeAliasType::clone() const {
  return new CppTypeAliasType(*this);
}

AbstractType* CppEnumeratorType::clone() const {
  return new CppEnumeratorType(*this);
}

AbstractType* CppEnumerationType::clone() const {
  return new CppEnumerationType(*this);
}

// AbstractType* CppArrayType::clone() const {
//   return new CppArrayType(*this);
// }

AbstractType* CppIntegralType::clone() const {
  return new CppIntegralType(*this);
}

bool CppIntegralType::moreExpressiveThan(CppIntegralType* rhs) const {
  TYPE_D(CppIntegralType);
  
  bool ret = d->m_type > rhs->d_func()->m_type && !((rhs->d_func()->m_modifiers & ModifierSigned) && !(d->m_modifiers & ModifierSigned));
  if((rhs->d_func()->m_modifiers & ModifierLongLong) && !(d->m_modifiers & ModifierLongLong))
    ret = false;
  if((rhs->d_func()->m_modifiers & ModifierLong) && !(d->m_modifiers & ModifierLongLong) && !(d->m_modifiers & ModifierLong))
    ret = false;
  return ret;
}

AbstractType* CppTemplateParameterType::clone() const {
  return new CppTemplateParameterType(*this);
}

bool CppCVType::equals(const CppCVType* rhs) const {
  return cvData()->m_constant == rhs->cvData()->m_constant && cvData()->m_volatile == rhs->cvData()->m_volatile;
}

bool CppFunctionType::equals(const AbstractType* _rhs) const
{
  if( !fastCast<const CppFunctionType*>(_rhs))
    return false;
  const CppFunctionType* rhs = static_cast<const CppFunctionType*>(_rhs);

  if( this == rhs )
    return true;
  
  //Ignore IdentifiedType here, because we do not want to respect that while comparing function-types.

  return CppCVType::equals(rhs) && FunctionType::equals(rhs);
}

bool CppPointerType::equals(const AbstractType* _rhs) const
{
  if( !fastCast<const CppPointerType*>(_rhs))
    return false;
  const CppPointerType* rhs = static_cast<const CppPointerType*>(_rhs);

  if( this == rhs )
    return true;

  return CppCVType::equals(rhs) && PointerType::equals(rhs);
}

bool CppReferenceType::equals(const AbstractType* _rhs) const
{
  if( !fastCast<const CppReferenceType*>(_rhs))
    return false;
  const CppReferenceType* rhs = static_cast<const CppReferenceType*>(_rhs);

  if( this == rhs )
    return true;
  
  return CppCVType::equals(rhs) && ReferenceType::equals(rhs);
}

bool CppClassType::equals(const AbstractType* _rhs) const
{
  if( !fastCast<const CppClassType*>(_rhs))
    return false;
  const CppClassType* rhs = fastCast<const CppClassType*>(_rhs);
  
  return IdentifiedType::equals(rhs) && CppCVType::equals(rhs) && StructureType::equals(rhs);
}

bool CppTypeAliasType::equals(const AbstractType* _rhs) const
{
  if( !fastCast<const CppTypeAliasType*>(_rhs))
    return false;
  const CppTypeAliasType* rhs = static_cast<const CppTypeAliasType*>(_rhs);

  if( this == rhs )
    return true;
  
  if( CppCVType::equals(rhs) && IdentifiedType::equals(rhs) )
  {
    if( (bool)d_func()->m_type != (bool)rhs->d_func()->m_type )
      return false;

    if( !d_func()->m_type )
      return true;
    
    return d_func()->m_type == rhs->d_func()->m_type;
  
  } else {
    return false;
  }
}

// QString CppEnumerationType::mangled() const
// {
//   return QString("E_%1_%2").arg(CppIntegralType::mangled()).arg(identifier().toString());
// }

bool CppEnumeratorType::equals(const AbstractType* _rhs) const
{
  const CppEnumeratorType* rhs = fastCast<const CppEnumeratorType*>(_rhs);
  
  if( !rhs )
    return false;

  if( this == rhs )
    return true;
  
  return IdentifiedType::equals(rhs) && CppConstantIntegralType::equals(rhs);
}

QString CppEnumeratorType::toString() const
{
  return "enum " + IdentifiedType::qualifiedIdentifier().toString();
}

uint CppEnumeratorType::hash() const
{
  return 27*(IdentifiedType::hash() + 13*CppConstantIntegralType::hash());
}

bool CppEnumerationType::equals(const AbstractType* _rhs) const
{
  const CppEnumerationType* rhs = fastCast<const CppEnumerationType*>(_rhs);
  if( !rhs )
    return false;

  if( this == rhs )
    return true;
  
  return IdentifiedType::equals(rhs) && CppIntegralType::equals(rhs);
}

// bool CppArrayType::equals(const AbstractType* _rhs) const
// {
//   if( !fastCast<const CppArrayType*>(_rhs))
//     return false;
//   const CppArrayType* rhs = static_cast<const CppArrayType*>(_rhs);
// 
//   if( this == rhs )
//     return true;
//   
//   return ArrayType::equals(rhs);
// }

bool CppIntegralType::equals(const AbstractType* _rhs) const
{
  TYPE_D(CppIntegralType);
  if( !fastCast<const CppIntegralType*>(_rhs) )
    return false;
  const CppIntegralType* rhs = static_cast<const CppIntegralType*>(_rhs);

  if( this == rhs )
    return true;
  
  return d->m_type == rhs->d_func()->m_type && d->m_modifiers == rhs->d_func()->m_modifiers && IntegralType::equals(rhs) && CppCVType::equals(rhs);
}

bool CppTemplateParameterType::equals(const AbstractType* _rhs) const
{
  if( !fastCast<const CppTemplateParameterType*>(_rhs))
    return false;
  const CppTemplateParameterType* rhs = static_cast<const CppTemplateParameterType*>(_rhs);

  if( this == rhs )
    return true;
  
  return IdentifiedType::equals(rhs);
}

void CppClassType::accept0 (TypeVisitor *v) const
{
  v->visit (this);
  v->endVisit (this);
}

void CppClassType::exchangeTypes(TypeExchanger */*e*/)
{
}

void CppClassType::setClassType(ClassType type)
{
  d_func_dynamic()->m_classType = type;
}

ClassType CppClassType::classType() const
{
  return d_func()->m_classType;
}
// ---------------------------------------------------------------------------
AbstractType::Ptr CppTypeAliasType::type() const
{
  return d_func()->m_type.type();
}

void CppTypeAliasType::setType(AbstractType::Ptr type)
{
  d_func_dynamic()->m_type = type->indexed();
}

// ---------------------------------------------------------------------------
/*QString CppEnumeratorType::value() const
{
  return m_value;
}

void CppEnumeratorType::setValue(const QString &value)
{
  m_value = value;
}*/

CppClassType::CppClassType(Declaration::CVSpecs spec) : CppClassTypeBase(createData<CppClassTypeData>()) {
  d_func_dynamic()->setTypeClassId<CppClassType>();
  CppCVType::setCV(spec);
}


CppConstantIntegralType::CppConstantIntegralType(IntegralTypes type, TypeModifiers modifiers) : CppIntegralType(createData<CppConstantIntegralTypeData>()) {
  d_func_dynamic()->setTypeClassId<CppConstantIntegralType>();
  
  setIntegralType(type);
  setTypeModifiers(modifiers);
}

template<>
void CppConstantIntegralType::setValueInternal<qint64>(qint64 value) {
  if((typeModifiers() & ModifierUnsigned))
    kDebug() << "setValue(signed) called on unsigned type";
  d_func_dynamic()->m_value = value;
}

template<>
void CppConstantIntegralType::setValueInternal<quint64>(quint64 value) {
  if(!(typeModifiers() & ModifierUnsigned))
    kDebug() << "setValue(unsigned) called on not unsigned type";
  d_func_dynamic()->m_value = (qint64)value;
}

template<>
void CppConstantIntegralType::setValueInternal<float>(float value) {
  if(integralType() != TypeFloat)
    kDebug() << "setValue(float) called on non-float type";
  memcpy(&d_func_dynamic()->m_value, &value, sizeof(float));
}

template<>
void CppConstantIntegralType::setValueInternal<double>(double value) {
  if(integralType() != TypeDouble)
    kDebug() << "setValue(double) called on non-double type";
  memcpy(&d_func_dynamic()->m_value, &value, sizeof(double));
}

uint CppConstantIntegralType::hash() const {
  uint ret = CppIntegralType::hash();
  ret += 47 * (uint)d_func()->m_value;
  return ret;
}

bool CppConstantIntegralType::equals(const KDevelop::AbstractType* _rhs) const {
  const CppConstantIntegralType* rhs = fastCast<const CppConstantIntegralType*>(_rhs);
  if(!rhs)
    return false;
  return d_func()->m_value == rhs->d_func()->m_value && IntegralType::equals(rhs);
}


QString CppConstantIntegralType::toString() const {
  switch(integralType()) {
    case TypeNone:
      return "none";
    case TypeChar:
      return QString("%1").arg((char)d_func()->m_value);
    case TypeWchar_t:
      return QString("%1").arg((wchar_t)d_func()->m_value);
    case TypeBool:
      return d_func()->m_value ? "true" : "false";
    case TypeInt:
      return (typeModifiers() & ModifierUnsigned) ? QString("%1u").arg((uint)d_func()->m_value) : QString("%1").arg((int)d_func()->m_value);
    case TypeFloat:
      return QString("%1").arg( value<float>() );
    case TypeDouble:
      return QString("%1").arg( value<double>() );
    case TypeVoid:
      return "void";
  }
  return "<unknown_value>";
}

CppIntegralType::CppIntegralType(IntegralTypes type, TypeModifiers modifiers) : MergeCppCVType< KDevelop::IntegralType >(createData<CppIntegralTypeData>())
{
  TYPE_D_DYNAMIC(CppIntegralType);
  d->setTypeClassId<CppIntegralType>();
  d->m_type = type;
  d->m_modifiers = modifiers;
  
  QString name;

  switch (type) {
    case TypeChar:
      name = "char";
      break;
    case TypeWchar_t:
      name = "wchar_t";
      break;
    case TypeBool:
      name = "bool";
      break;
    case TypeInt:
      name = "int";
      break;
    case TypeFloat:
      name = "float";
      break;
    case TypeDouble:
      name = "double";
      break;
    case TypeVoid:
      name = "void";
      break;
    default:
      name = "<unknown>";
      break;
  }

  if (modifiers & ModifierUnsigned)
    name.prepend("unsigned ");
  else if (modifiers & ModifierSigned)
    name.prepend("signed ");

  if (modifiers & ModifierShort)
    name.prepend("short ");
  else if (modifiers & ModifierLong)
    name.prepend("long ");

  setName(IndexedString(name));
}

TypeModifiers CppIntegralType::typeModifiers() const
{
  return d_func()->m_modifiers;
}

IntegralTypes CppIntegralType::integralType() const
{
  return d_func()->m_type;
}

QString CppIntegralType::toString() const
{
  return QString("%1%2").arg(cvString()).arg(IntegralType::toString());
}

void CppCVType::clear() {
  cvData()->m_constant = false;
  cvData()->m_volatile = false;
}


// CppCVType::CppCVType(/*Declaration::CVSpecs spec*/)
// {
//   cvData()->m_constant = spec & Declaration::Const;
//   cvData()->m_volatile = spec & Declaration::Volatile;
// }

QString CppCVType::cvString() const
{
  return QString("%1%2").arg(isConstant() ? " const " : "").arg(isVolatile() ? " volatile " : "");
}

QString CppFunctionType::toString() const
{
  return QString("%1 %2").arg(FunctionType::toString()).arg(cvString());
}

CppPointerType::CppPointerType(Declaration::CVSpecs spec) : CppPointerTypeBase(createData<CppPointerTypeData>())
{
  d_func_dynamic()->setTypeClassId<CppPointerType>();
  setCV(spec);
}

QString CppPointerType::toString() const
{
  return QString("%1%2").arg(PointerType::toString()).arg(cvString());
}

CppReferenceType::CppReferenceType(Declaration::CVSpecs spec) : CppReferenceTypeBase(createData<CppReferenceTypeData>())
{
  d_func_dynamic()->setTypeClassId<CppReferenceType>();
  setCV(spec);
}

QString CppReferenceType::toString() const
{
  return QString("%1%2").arg(ReferenceType::toString()).arg(cvString());
}

/*CppArrayType::CppArrayType(Declaration::CVSpecs spec)
  : CppCVType(spec)
{
}

QString CppArrayType::toString() const
{
  return QString("%1%2").arg(cvString()).arg(ArrayType::toString());
}*/

uint CppIntegralType::hash() const
{
  return cvHash(IntegralType::hash()) + + 11*(uint)integralType()+83*(uint)typeModifiers();
}

uint CppPointerType::hash() const
{
  return cvHash(PointerType::hash());
}

uint CppReferenceType::hash() const
{
  return cvHash(ReferenceType::hash());
}

uint CppFunctionType::hash() const
{
  return cvHash(FunctionType::hash());
}

uint CppClassType::hash() const
{
  return cvHash(IdentifiedType::hash() + 19*StructureType::hash());
}

uint CppTypeAliasType::hash() const
{
  return cvHash(31 * IdentifiedType::hash() + (type() ? type()->hash() + 83 : 0));
}

QString CppTypeAliasType::toString() const
{
  QualifiedIdentifier id = qualifiedIdentifier();
  if (!id.isEmpty())
    return id.top().toString();

  if (type())
    return type()->toString();

  return "typedef <notype>";
}

void CppClassType::clear() {
  //StructureType::clear();
  IdentifiedType::clear();
  CppCVType::clear();
  d_func_dynamic()->m_classType = Class;
  d_func_dynamic()->m_closed = false;
}

QString CppClassType::toString() const
{
  QualifiedIdentifier id = qualifiedIdentifier();
  if (!id.isEmpty())
    return id.top().toString();

  QString type;
  switch (classType()) {
    case Class:
      type = "class";
      break;
    case Struct:
      type = "struct";
      break;
    case Union:
      type = "union";
      break;
  }
  return QString("<%1>%2").arg(type).arg(cvString());
}

CppEnumerationType::CppEnumerationType(Declaration::CVSpecs spec) : CppEnumerationTypeBase(createData<CppEnumerationTypeData>())
{
  d_func_dynamic()->setTypeClassId<CppEnumerationType>();
  CppIntegralType::setIntegralType(TypeInt);
  setCV(spec);
}

QString CppEnumerationType::toString() const
{
  return "enum " + qualifiedIdentifier().toString();
}
uint CppEnumerationType::hash() const
{
  return IdentifiedType::hash() + 57*CppIntegralType::hash();
}

QString CppCVType::cvMangled() const
{
  QString ret;
  if (cvData()->m_constant)
    ret += 'C';
  if (cvData()->m_volatile)
    ret += 'V';
  return ret;
}

TemplateParameterDeclaration* CppTemplateParameterType::declaration(const TopDUContext* top) const {
  return static_cast<TemplateParameterDeclaration*>(IdentifiedType::declaration(top));
}

QString CppTemplateParameterType::toString() const {
  return "<template> " + IdentifiedType::qualifiedIdentifier().toString();
}

void CppTemplateParameterType::accept0 (KDevelop::TypeVisitor *v) const {
    v->visit(this);
/*    v->endVisit(this);*/
}

uint CppTemplateParameterType::hash() const {
  return 41*IdentifiedType::hash();
}

TypeModifiers& operator|=(TypeModifiers& lhs, const TypeModifiers& rhs) {
  lhs = (TypeModifiers)((uint)lhs | (uint)rhs);
  return lhs;
}

TypeModifiers operator|(const TypeModifiers& lhs, const TypeModifiers& rhs) {
  return (TypeModifiers)((uint)lhs | (uint)rhs);
}

