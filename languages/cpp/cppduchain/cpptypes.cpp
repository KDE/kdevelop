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
#include <forwarddeclarationtype.h>
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

bool CppIntegralType::moreExpressiveThan(CppIntegralType* rhs) const {
  bool ret = m_type > rhs->m_type && !((rhs->m_modifiers & ModifierSigned) && !(m_modifiers & ModifierSigned));
  if((rhs->m_modifiers & ModifierLongLong) && !(m_modifiers & ModifierLongLong))
    ret = false;
  if((rhs->m_modifiers & ModifierLong) && !(m_modifiers & ModifierLongLong) && !(m_modifiers & ModifierLong))
    ret = false;
  return ret;
}

AbstractType* CppTemplateParameterType::clone() const {
  return new CppTemplateParameterType(*this);
}

bool CppCVType::equals(const CppCVType* rhs) const {
  return m_constant == rhs->m_constant && m_volatile == rhs->m_volatile;
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
  if( !fastCast<const CppClassType*>(_rhs) && !fastCast<const ForwardDeclarationType*>(_rhs))
    return false;
  const IdentifiedType* rhs = fastCast<const IdentifiedType*>(_rhs);
  
  Declaration* decl = declaration();
  Declaration* rhsDecl = rhs->declaration();
  
  if(!decl || !rhsDecl)
    return false;

  ///We cannot use here IdentifiedType::equals, because else we get problems with forward declarations
  return decl->equalQualifiedIdentifier(rhsDecl);
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
    if( (bool)m_type != (bool)rhs->m_type )
      return false;

    if( !m_type )
      return true;
    
    return m_type->equals(rhs->m_type.data());
  
  } else {
    return false;
  }
}

QString CppEnumerationType::mangled() const
{
  return QString("E_%1_%2").arg(CppIntegralType::mangled()).arg(identifier().toString());
}

bool CppEnumerationType::equals(const AbstractType* _rhs) const
{
  if( !fastCast<const CppEnumerationType*>(_rhs) && !fastCast<const ForwardDeclarationType*>(_rhs))
    return false;
  const IdentifiedType* rhs = fastCast<const IdentifiedType*>(_rhs);

  if( this == rhs )
    return true;
  
  return IdentifiedType::equals(rhs);
}

bool CppArrayType::equals(const AbstractType* _rhs) const
{
  if( !fastCast<const CppArrayType*>(_rhs))
    return false;
  const CppArrayType* rhs = static_cast<const CppArrayType*>(_rhs);

  if( this == rhs )
    return true;
  
  return ArrayType::equals(rhs);
}

bool CppIntegralType::equals(const AbstractType* _rhs) const
{
  if( !fastCast<const CppIntegralType*>(_rhs))
    return false;
  const CppIntegralType* rhs = static_cast<const CppIntegralType*>(_rhs);

  if( this == rhs )
    return true;
  
  return m_type == rhs->m_type && m_modifiers == rhs->m_modifiers && IntegralType::equals(rhs) && CppCVType::equals(rhs);
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

CppEnumeratorType::CppEnumeratorType() : CppConstantIntegralType(TypeInt)
{
  setCV(KDevelop::Declaration::Const);
}

CppClassType::CppClassType(Declaration::CVSpecs spec)
  : CppCVType(spec)
  , m_classType(Class)
  , m_closed(false)
{
}


CppConstantIntegralType::CppConstantIntegralType(IntegralTypes type, CppConstantIntegralType::TypeModifiers modifiers) : CppIntegralType(type, modifiers) {
}

template<>
void CppConstantIntegralType::setValueInternal<qint64>(qint64 value) {
  if((typeModifiers() & ModifierUnsigned))
    kDebug() << "setValue(signed) called on unsigned type";
  m_value = value;
}

template<>
void CppConstantIntegralType::setValueInternal<quint64>(quint64 value) {
  if(!(typeModifiers() & ModifierUnsigned))
    kDebug() << "setValue(unsigned) called on not unsigned type";
  m_value = (qint64)value;
}

template<>
void CppConstantIntegralType::setValueInternal<float>(float value) {
  if(integralType() != TypeFloat)
    kDebug() << "setValue(float) called on non-float type";
  memcpy(&m_value, &value, sizeof(float));
}

template<>
void CppConstantIntegralType::setValueInternal<double>(double value) {
  if(integralType() != TypeDouble)
    kDebug() << "setValue(double) called on non-double type";
  memcpy(&m_value, &value, sizeof(double));
}

uint CppConstantIntegralType::hash() const {
  uint ret = IntegralType::hash();
  ret += 47 * (uint)m_value + 11*(uint)integralType();
  return ret;
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

uint CppIntegralType::hash() const
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
  return cvHash(FunctionType::hash()) + 31 * identifier().hash();
}

uint CppClassType::hash() const
{
  return identifier().hash();

/*  foreach (const BaseClassInstance& base, m_baseClasses)
    hash_val = (hash_val << 5) - hash_val + base.baseClass->hash() + (base.access + base.virtualInheritance) * 281;

  return hash_val;*/
}

uint CppTypeAliasType::hash() const
{
  return 31 * identifier().hash() + (type() ? type()->hash() + 83 : 0);
}

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

QString CppEnumerationType::toString() const
{
  return "enum " + identifier().toString();
}
uint CppEnumerationType::hash() const
{
  return identifier().hash();
}

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
  return ret + cvMangled();
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
  QString ret = "P" + cvMangled();
  if (baseType())
    ret += baseType()->mangled();
  return ret;
}

QString CppReferenceType::mangled() const
{
  QString ret = "R" + cvMangled();
  if (baseType())
    ret += baseType()->mangled();
  return ret;
}

QString CppClassType::mangled() const
{
  return idMangled() + cvMangled();
}

QString CppTypeAliasType::mangled() const
{
  if( type() )
    return "TA" + type()->mangled() + cvMangled();
  else
    return "TA" + cvMangled();
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

uint CppTemplateParameterType::hash() const {
  return 41*identifier().hash();
}

