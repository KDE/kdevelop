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
#include "templateparameterdeclaration.h"
using namespace KDevelop;

//Because all these classes have no d-pointers, shallow copies are perfectly fine

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

AbstractType* CppEnumerationType::clone() const {
  return new CppEnumerationType(*this);
}

AbstractType* CppArrayType::clone() const {
  return new CppArrayType(*this);
}

AbstractType* CppIntegralType::clone() const {
  return new CppIntegralType(*this);
}

AbstractType* CppTemplateParameterType::clone() const {
  return new CppTemplateParameterType(*this);
}

bool CppCVType::equals(const CppCVType* rhs) const {
  return m_constant == rhs->m_constant && m_volatile == rhs->m_volatile;
}

bool CppFunctionType::equals(const AbstractType* _rhs) const
{
  if( !dynamic_cast<const CppFunctionType*>(_rhs))
    return false;
  const CppFunctionType* rhs = static_cast<const CppFunctionType*>(_rhs);

  if( this == rhs )
    return true;
  
  //Ignore IdentifiedType here, because we do not want to respect that while comparing function-types.

  return CppCVType::equals(rhs) && FunctionType::equals(rhs);
}

bool CppPointerType::equals(const AbstractType* _rhs) const
{
  if( !dynamic_cast<const CppPointerType*>(_rhs))
    return false;
  const CppPointerType* rhs = static_cast<const CppPointerType*>(_rhs);

  if( this == rhs )
    return true;

  return CppCVType::equals(rhs) && PointerType::equals(rhs);
}

bool CppReferenceType::equals(const AbstractType* _rhs) const
{
  if( !dynamic_cast<const CppReferenceType*>(_rhs))
    return false;
  const CppReferenceType* rhs = static_cast<const CppReferenceType*>(_rhs);

  if( this == rhs )
    return true;
  
  return CppCVType::equals(rhs) && ReferenceType::equals(rhs);
}

bool CppClassType::equals(const AbstractType* _rhs) const
{
  if( !dynamic_cast<const CppClassType*>(_rhs))
    return false;
  const CppClassType* rhs = static_cast<const CppClassType*>(_rhs);

  return identifier() == rhs->identifier();
  
/*  if( this == rhs )
    return true;

  if( m_classType != rhs->m_classType )
    return false;

  if( m_baseClasses.count() != rhs->m_baseClasses.count() )
    return false;

  QList<BaseClassInstance>::const_iterator it1 = m_baseClasses.begin();
  QList<BaseClassInstance>::const_iterator it2 = rhs->m_baseClasses.begin();

  for( ;it1 != m_baseClasses.end(); ++it1, ++it2 ) {
    if( (bool)it1->baseClass != (bool)it2->baseClass )
      return false;
    if( it1->access != it2->access )
      return false;

    if( !it1->baseClass)
      continue;

    if( !it1->baseClass->equals( it2->baseClass.data() ) )
      return false;
  }
  
  return CppCVType::equals(rhs) && IdentifiedType::equals(rhs) && StructureType::equals(rhs);*/
}

bool CppTypeAliasType::equals(const AbstractType* _rhs) const
{
  if( !dynamic_cast<const CppTypeAliasType*>(_rhs))
    return false;
  const CppTypeAliasType* rhs = static_cast<const CppTypeAliasType*>(_rhs);

  if( this == rhs )
    return true;
  
  if( CppCVType::equals(rhs) && IdentifiedType::equals(rhs) )
  {
    if( (bool)m_type != (bool)rhs->m_type )
      return false;

    if( !m_type )
      return true;
    
    return m_type->equals(rhs->m_type.data());
  
  } else {
    return false;
  }
}

bool CppEnumerationType::equals(const AbstractType* _rhs) const
{
  if( !dynamic_cast<const CppEnumerationType*>(_rhs))
    return false;
  const CppEnumerationType* rhs = static_cast<const CppEnumerationType*>(_rhs);

  if( this == rhs )
    return true;
  
  return CppIntegralType::equals(rhs) && IdentifiedType::equals(rhs);
}

bool CppArrayType::equals(const AbstractType* _rhs) const
{
  if( !dynamic_cast<const CppArrayType*>(_rhs))
    return false;
  const CppArrayType* rhs = static_cast<const CppArrayType*>(_rhs);

  if( this == rhs )
    return true;
  
  return ArrayType::equals(rhs);
}

bool CppIntegralType::equals(const AbstractType* _rhs) const
{
  if( !dynamic_cast<const CppIntegralType*>(_rhs))
    return false;
  const CppIntegralType* rhs = static_cast<const CppIntegralType*>(_rhs);

  if( this == rhs )
    return true;
  
  return m_type == rhs->m_type && m_modifiers == rhs->m_modifiers && IntegralType::equals(rhs) && CppCVType::equals(rhs);
}

bool CppTemplateParameterType::equals(const AbstractType* _rhs) const
{
  if( !dynamic_cast<const CppTemplateParameterType*>(_rhs))
    return false;
  const CppTemplateParameterType* rhs = static_cast<const CppTemplateParameterType*>(_rhs);

  if( this == rhs )
    return true;
  
  return IdentifiedType::equals(rhs);
}

void CppClassType::accept0 (TypeVisitor *v) const
{
  if (v->visit (this))
    {
      foreach(const BaseClassInstance& base, m_baseClasses)
        acceptType (AbstractType::Ptr( const_cast<AbstractType*>( static_cast<const AbstractType*>(base.baseClass.data()) ) ), v);
    }

  v->endVisit (this);
}

void CppClassType::exchangeTypes(TypeExchanger *e)
{
  for(QList<BaseClassInstance>::iterator it = m_baseClasses.begin(); it != m_baseClasses.end(); ++it )
    (*it).baseClass = e->exchange((*it).baseClass.data());
}


// ---------------------------------------------------------------------------
const QList<CppClassType::BaseClassInstance>& CppClassType::baseClasses() const
{
  return m_baseClasses;
}

void CppClassType::addBaseClass(const BaseClassInstance& baseClass)
{
  m_baseClasses.append(baseClass);
}

void CppClassType::removeBaseClass(AbstractType::Ptr baseClass)
{
  for (int i = 0; i < m_baseClasses.count(); ++i)
    if (m_baseClasses[i].baseClass == baseClass) {
      m_baseClasses.removeAt(i);
      return;
    }
}

void CppClassType::setClassType(ClassType type)
{
  m_classType = type;
}

CppClassType::ClassType CppClassType::classType() const
{
  return m_classType;
}

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
AbstractType::Ptr CppTypeAliasType::type() const
{
  return m_type;
}

void CppTypeAliasType::setType(AbstractType::Ptr type)
{
  m_type = type;
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

CppTypeAliasType::CppTypeAliasType()
{
}

/*CppEnumeratorType::CppEnumeratorType()
{
}*/

CppClassType::CppClassType(Declaration::CVSpecs spec)
  : CppCVType(spec)
  , m_classType(Class)
  , m_closed(false)
{
}


CppConstantIntegralType::CppConstantIntegralType(IntegralTypes type, CppConstantIntegralType::TypeModifiers modifiers) : CppIntegralType(type, modifiers) {
}

template<>
void CppConstantIntegralType::setValue<long long>(long long value) {
  if((typeModifiers() & ModifierUnsigned))
    kWarning() << "setValue(signed) called on unsigned type";
  m_value = value;
}

template<>
void CppConstantIntegralType::setValue<unsigned long long>(unsigned long long value) {
  if(!(typeModifiers() & ModifierUnsigned))
    kWarning() << "setValue(unsigned) called on not unsigned type";
  m_value = (long long)value;
}

template<>
void CppConstantIntegralType::setValue<float>(float value) {
  if(integralType() != TypeFloat)
    kWarning() << "setValue(float) called on non-float type";
  memcpy(&m_value, &value, sizeof(float));
}

template<>
void CppConstantIntegralType::setValue<double>(double value) {
  if(integralType() != TypeDouble)
    kWarning() << "setValue(double) called on non-double type";
  memcpy(&m_value, &value, sizeof(double));
}

template<>
long long CppConstantIntegralType::value<long long>() const {
  if((typeModifiers() & ModifierUnsigned))
    kWarning() << "value<signed> called on unsigned type";
  return m_value;
}

template<>
unsigned long long CppConstantIntegralType::value<unsigned long long>() const {
  if(!(typeModifiers() & ModifierUnsigned))
    kWarning() << "value<unsigned> called on not unsigned type";
  return (unsigned long long)m_value;
}

template<>
float CppConstantIntegralType::value<float>() const {
  if(integralType() != TypeFloat)
    kWarning() << "value<float> called on non-float type";

  float value;
  memcpy(&value, &m_value, sizeof(float));
  return value;
}

template<>
double CppConstantIntegralType::value<double>() const {
  if(integralType() != TypeDouble)
    kWarning() << "value<double> called on non-double type";

  double value;
  memcpy(&value, &m_value, sizeof(double));
  return value;
}

QString CppConstantIntegralType::toString() const {
  switch(integralType()) {
    case TypeNone:
      return "none";
    case TypeChar:
      return QString("%1").arg((char)m_value);
    case TypeWchar_t:
      return QString("%1").arg((wchar_t)m_value);
    case TypeBool:
      return m_value ? "true" : "false";
    case TypeInt:
      return (typeModifiers() & ModifierUnsigned) ? QString("%1u").arg((uint)m_value) : QString("%1").arg((int)m_value);
    case TypeFloat:
      return QString("%1").arg( value<float>() );
    case TypeDouble:
      return QString("%1").arg( value<double>() );
    case TypeVoid:
      return "void";
  }
  return "<unknown_value>";
}

CppIntegralType::CppIntegralType(IntegralTypes type, CppIntegralType::TypeModifiers modifiers)
  : m_type(type)
  , m_modifiers(modifiers)
{
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
      name = "<notype>";
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

  setName(name);
}

CppIntegralType::TypeModifiers CppIntegralType::typeModifiers() const
{
  return m_modifiers;
}

CppIntegralType::IntegralTypes CppIntegralType::integralType() const
{
  return m_type;
}

QString CppIntegralType::toString() const
{
  return QString("%1%2").arg(cvString()).arg(IntegralType::toString());
}

void CppCVType::clear() {
  m_constant = false;
  m_volatile = false;
}


CppCVType::CppCVType(Declaration::CVSpecs spec)
  : m_constant(spec & Declaration::Const)
  , m_volatile(spec & Declaration::Volatile)
{
}

QString CppCVType::cvString() const
{
  return QString("%1%2").arg(isConstant() ? " const " : "").arg(isVolatile() ? " volatile " : "");
}

QString CppFunctionType::toString() const
{
  return QString("%1 %2").arg(FunctionType::toString()).arg(cvString());
}

CppPointerType::CppPointerType(Declaration::CVSpecs spec)
  : CppCVType(spec)
{
}

QString CppPointerType::toString() const
{
  return QString("%1%2").arg(PointerType::toString()).arg(cvString());
}

CppReferenceType::CppReferenceType(Declaration::CVSpecs spec)
  : CppCVType(spec)
{
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

/*uint CppIntegralType::hash() const
{
  return cvHash(IntegralType::hash());
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
  uint hash_value = cvHash(StructureType::hash());

  foreach (const BaseClassInstance& base, m_baseClasses)
    hash_value = (hash_value << 5) - hash_value + base.baseClass->hash() + (base.access + base.virtualInheritance) * 281;

  return hash_value;
}

uint CppTypeAliasType::hash() const
{
  return type() ? type()->hash() + 83 : 0;
}*/

QString CppTypeAliasType::toString() const
{
  QualifiedIdentifier id = identifier();
  if (!id.isEmpty())
    return id.top().toString();

  if (type())
    return type()->toString();

  return "typedef <notype>";
}

void CppClassType::addElement(AbstractType::Ptr element)
{
  if (isClosed())
    kWarning(9007) << "Tried to add type" << element->toString() << "to closed class!" ;
  else
    StructureType::addElement(element);
}

void CppClassType::clear() {
  StructureType::clear();
  IdentifiedType::clear();
  CppCVType::clear();
  m_baseClasses.clear();
  m_classType = Class;
  m_closed = false;
}

QString CppClassType::toString() const
{
  QualifiedIdentifier id = identifier();
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

CppEnumerationType::CppEnumerationType(Declaration::CVSpecs spec)
  : CppIntegralType(TypeInt)
{
  setCV(spec);
}

/*uint CppEnumerationType::hash() const
{
  return reinterpret_cast<long>(this);
}*/

QString CppIntegralType::mangled() const
{
  QString ret;
  if (typeModifiers() & ModifierUnsigned)
    ret = "U";

  switch (integralType()) {
    case TypeChar:
      if (typeModifiers() & ModifierSigned)
        ret += 'S';
      ret += 'c';
      break;
    case TypeWchar_t:
      ret += 'w';
      break;
    case TypeBool:
      ret += 'b';
      break;
    case TypeInt:
      if (typeModifiers() & ModifierLong)
        if (typeModifiers() & ModifierLongLong)
          ret += 'x';
        else
          ret += 'l';
      else
        ret += 'i';
      break;
    case TypeFloat:
      ret += 'f';
      break;
    case TypeDouble:
      if (typeModifiers() & ModifierLong)
        ret += 'r';
      else
        ret += 'd';
      break;
    case TypeVoid:
      ret += 'v';
      break;
    default:
      ret += '?';
      break;
  }
  return ret;
}

QString CppCVType::cvMangled() const
{
  QString ret;
  if (m_constant)
    ret += 'C';
  if (m_volatile)
    ret += 'V';
  return ret;
}

QString CppPointerType::mangled() const
{
  QString ret = "P";
  if (baseType())
    ret += baseType()->mangled();
  return ret;
}

QString CppReferenceType::mangled() const
{
  QString ret = "R";
  if (baseType())
    ret += baseType()->mangled();
  return ret;
}

QString CppClassType::mangled() const
{
  return idMangled();
}

QString CppTypeAliasType::mangled() const
{
  if( type() )
    return "TA" + type()->mangled();
  else
    return "TA";
}

bool CppFunctionType::isTemplate() const {
  return false;
}

bool CppFunctionType::isMoreSpecialized( const CppFunctionType* other ) const {
  Q_UNUSED(other)
  return false;
}

QString CppFunctionType::mangled() const
{
  ClassFunctionDeclaration* classFunctionDecl = dynamic_cast<ClassFunctionDeclaration*>(declaration());

  bool constructor = classFunctionDecl && classFunctionDecl->isConstructor();

  QualifiedIdentifier id = identifier();

  Identifier function = id.top();
  if (!id.isEmpty())
    id.pop();

  QString ret = QString("%1__%2%3").arg(constructor ? QString() : function.mangled()).arg(cvMangled()).arg(id.mangled());

  foreach (const AbstractType::Ptr& argument, arguments())
    if (argument)
      ret += argument->mangled();
    else
      ret += '?';

  return ret;
}

QString CppArrayType::mangled() const
{
  return QString("A%1%2").arg(dimension()).arg(elementType() ? elementType()->mangled() : QString());
}

QString CppEnumerationType::mangled() const
{
  return idMangled();
}

TemplateParameterDeclaration* CppTemplateParameterType::declaration() const {
  return static_cast<TemplateParameterDeclaration*>(IdentifiedType::declaration());
}

QString CppTemplateParameterType::toString() const {
  return "<template> " + (declaration() ? declaration()->identifier().toString() : QString());
}

QString CppTemplateParameterType::mangled() const
{
  return QString("T%1").arg(declaration() ? declaration()->identifier().toString() : QString());
}

void CppTemplateParameterType::accept0 (KDevelop::TypeVisitor *v) const {
    v->visit(this);
/*    v->endVisit(this);*/
}
