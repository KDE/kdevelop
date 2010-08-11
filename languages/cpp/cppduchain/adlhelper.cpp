/*
    Copyright (C) 2010 Ciprian Ciubotariu <cheepeero@gmx.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "adlhelper.h"
#include <language/duchain/types/alltypes.h>
#include <language/duchain/classdeclaration.h>

using namespace Cpp;
using namespace KDevelop;

ADLTypeVisitor::ADLTypeVisitor(ADLHelper & helper) : m_helper(helper)
{
}

bool ADLTypeVisitor::preVisit(const AbstractType * type)
{
  // the following types are of no interest to ADL
  switch (type->whichType())
  {
  case AbstractType::TypeAbstract:
  case AbstractType::TypeIntegral:
  case AbstractType::TypeDelayed:
  case AbstractType::TypeUnsure:
    return false;
  default:
    return true;
  };
}

void ADLTypeVisitor::postVisit(const AbstractType *)
{
}

bool ADLTypeVisitor::visit(const AbstractType* type)
{
  return !seen(type);
}

void ADLTypeVisitor::visit(const IntegralType *)
{
}

bool ADLTypeVisitor::visit(const PointerType * type)
{
  return !seen(type);
}

void ADLTypeVisitor::endVisit(const PointerType *)
{
  // traversed by PointerType::accept0
}

bool ADLTypeVisitor::visit(const ReferenceType * type)
{
  return !seen(type);
}

void ADLTypeVisitor::endVisit(const ReferenceType *)
{
  // traversed by ReferenceType::accept0
}

bool ADLTypeVisitor::visit(const FunctionType * type)
{
  return !seen(type);
}

void ADLTypeVisitor::endVisit(const FunctionType *)
{
  // return type and argument types are handled by FunctionType::accept0
}

bool ADLTypeVisitor::visit(const StructureType * type)
{
  return !seen(type);
}

void ADLTypeVisitor::endVisit(const StructureType * type)
{
  // StructureType does not visit base classes etc
  // so the processing is done by ADLHelper
  m_helper.addAssociatedClass(type->declaration(m_helper.m_topContext.data()));
}

bool ADLTypeVisitor::visit(const ArrayType * type)
{
  return !seen(type);
}

void ADLTypeVisitor::endVisit(const ArrayType *)
{
  // traversed by ArrayType::accept0
}

bool ADLTypeVisitor::seen(const KDevelop::AbstractType* type)
{
  if (m_seen.contains(type))
    return true;

  m_seen.insert(type);
  return false;
}


ADLHelper::ADLHelper(DUContextPointer context, TopDUContextPointer topContext)
    : m_context(context), m_topContext(topContext), m_typeVisitor(*this)
{
}

void ADLHelper::addArgument(const OverloadResolver::Parameter & argument)
{
  addArgumentType(argument.type);
}

void ADLHelper::addArgumentType(const AbstractType::Ptr typePtr)
{
  if (typePtr)
  {
    // the enumeration and enumerator types are not part of the TypeVisitor interface
    switch (typePtr->whichType())
    {
    case AbstractType::TypeEnumeration:
      {
        EnumerationType* specificType = fastCast<EnumerationType*>(typePtr.unsafeData());
        if (specificType)
        {
          Declaration * enumDecl = specificType->declaration(m_topContext.data());
          addDeclarationScopeIdentifier(enumDecl);
        }
        break;
      }
    case AbstractType::TypeEnumerator:
      {
        EnumeratorType* specificType = fastCast<EnumeratorType*>(typePtr.unsafeData());
        if (specificType)
        {
          // use the enumeration context for the enumerator value declaration to find out the namespace
          Declaration * enumeratorDecl = specificType->declaration(m_topContext.data());
          DUContext * enumContext = enumeratorDecl->context();
          addAssociatedNamespace(enumContext->scopeIdentifier(false));
        }
        break;
      }
    default:
      typePtr->accept(&m_typeVisitor);
    }
  }
}

QSet< Declaration* > ADLHelper::associatedNamespaces() const
{
  return m_associatedNamespaces;
}

void ADLHelper::addAssociatedClass(Declaration * declaration)
{
  if (!declaration || !m_context || !m_topContext)
    return;

  // from the standard:
  // Typedef names and using-declarations used to specify the types do not contribute to this set.
  if (declaration->isTypeAlias())
    return;

  QList<Declaration*> associatedClasses;
  associatedClasses << declaration;

  QList<Declaration*> baseClasses = computeAllBaseClasses(declaration);
  associatedClasses << baseClasses;

  // no need to search for parent class, since scopeIdentifier() below skips them anyway

  foreach(Declaration * decl, associatedClasses)
  {
    addDeclarationScopeIdentifier(decl);
  }
}

void ADLHelper::addDeclarationScopeIdentifier(Declaration * decl)
{
  if (decl)
  {
    DUContext* declContext = decl->logicalInternalContext(m_topContext.data());
    if (declContext)
      addAssociatedNamespace(declContext->scopeIdentifier(false));
  }
}

void ADLHelper::addAssociatedNamespace(const QualifiedIdentifier & identifier)
{
  QList<Declaration*> decls = m_context->findDeclarations(identifier , KDevelop::SimpleCursor(), AbstractType::Ptr(), m_topContext.data());
  foreach(Declaration * decl, decls)
  {
    addAssociatedNamespace(decl);
  }
}

void ADLHelper::addAssociatedNamespace(Declaration * declaration)
{
  if (!declaration)
    return;

  if (declaration->kind() == Declaration::Namespace)
  {
    //kDebug() << "adding namespace " << declaration->toString();
    m_associatedNamespaces += declaration;
  }
}

QList<Declaration *> ADLHelper::computeAllBaseClasses(Declaration* declaration)
{
  QList<Declaration *> baseClasses;

  if (declaration)
  {
    baseClasses << declaration;

    ClassDeclaration * classDecl = dynamic_cast<ClassDeclaration*>(declaration);
    if (classDecl)
    {
      int nBaseClassesCount = classDecl->baseClassesSize();
      for (int i = 0; i < nBaseClassesCount; ++i)
      {
        const BaseClassInstance baseClass = classDecl->baseClasses()[i];
        AbstractType::Ptr type = baseClass.baseClass.abstractType();
        if (type)
        {
          StructureType::Ptr structType = type.cast<StructureType>();
          if (structType)
          {
            Declaration * decl = structType->declaration(m_topContext.data());
            baseClasses << computeAllBaseClasses(decl);
          }
        }
      }
    }
  }

  return baseClasses;
}
