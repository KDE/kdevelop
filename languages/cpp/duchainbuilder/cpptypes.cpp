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

#include "cpptypes.h"

#include "classfunctiondeclaration.h"

// ---------------------------------------------------------------------------
const QList<CppClassType::BaseClassInstance>& CppClassType::baseClasses() const
{
  return m_baseClasses;
}

void CppClassType::addBaseClass(const BaseClassInstance& baseClass)
{
  m_baseClasses.append(baseClass);
}

void CppClassType::removeBaseClass(CppClassType::Ptr baseClass)
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

CppClassType::CppClassType(Cpp::CVSpecs spec)
  : CppCVType(spec)
  , m_classType(Class)
  , m_closed(false)
{
}

// kate: space-indent on; indent-width 2; replace-tabs on;

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

CppCVType::CppCVType(Cpp::CVSpecs spec)
  : m_constant(spec & Cpp::Const)
  , m_volatile(spec & Cpp::Volatile)
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

CppPointerType::CppPointerType(Cpp::CVSpecs spec)
  : CppCVType(spec)
{
}

QString CppPointerType::toString() const
{
  return QString("%1%2").arg(PointerType::toString()).arg(cvString());
}

CppReferenceType::CppReferenceType(Cpp::CVSpecs spec)
  : CppCVType(spec)
{
}

QString CppReferenceType::toString() const
{
  return QString("%1%2").arg(ReferenceType::toString()).arg(cvString());
}

/*CppArrayType::CppArrayType(Cpp::CVSpecs spec)
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
    kWarning() << k_funcinfo << "Tried to add type " << element->toString() << " to closed class!" << endl;
  else
    StructureType::addElement(element);
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

CppEnumerationType::CppEnumerationType(Cpp::CVSpecs spec)
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

CppIdentifiedType::CppIdentifiedType()
  : m_declaration(0)
{
}

QualifiedIdentifier CppIdentifiedType::identifier() const
{
  return m_declaration ? m_declaration->qualifiedIdentifier() : QualifiedIdentifier();
}

Declaration* CppIdentifiedType::declaration() const
{
  return m_declaration;
}

void CppIdentifiedType::setDeclaration(Declaration* declaration)
{
  m_declaration = declaration;
}

QString CppIdentifiedType::idMangled() const
{
  return identifier().mangled();
}

QString CppClassType::mangled() const
{
  return idMangled();
}

QString CppTypeAliasType::mangled() const
{
  return type()->mangled();
}

QString CppFunctionType::mangled() const
{
  bool constructor = declaration() && declaration()->isConstructor();

  QualifiedIdentifier id = identifier();

  Identifier function = id.top();
  if (!id.isEmpty())
    id.pop();

  QString ret = QString("%1__%2%3").arg(constructor ? QString() : function.mangled()).arg(cvMangled()).arg(id.mangled());

  foreach (const AbstractType::Ptr& argument, arguments())
    if (argument)
      ret += argument->mangled();
    else
      ret += "?";

  return ret;
}

ClassFunctionDeclaration* CppFunctionType::declaration() const
{
  return static_cast<ClassFunctionDeclaration*>(CppIdentifiedType::declaration());
}

void CppFunctionType::setDeclaration(ClassFunctionDeclaration* declaration)
{
  CppIdentifiedType::setDeclaration(declaration);
}

QString CppArrayType::mangled() const
{
  return QString("A%1%2").arg(dimension()).arg(elementType() ? elementType()->mangled() : QString());
}

QString CppEnumerationType::mangled() const
{
  return idMangled();
}
