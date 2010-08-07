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

using namespace Cpp;
using namespace KDevelop;

ADLHelper::ADLHelper( DUContextPointer context, TopDUContextPointer topContext )
    : m_context( context ), m_topContext( topContext )
{
}

void ADLHelper::addArgument( const OverloadResolver::Parameter & argument )
{
  addArgumentType( argument.type );
}

void ADLHelper::addArgumentType( const AbstractType::Ptr type )
{
  // TODO: refactor into a visitor
  if ( type )
  {
    switch ( type->whichType() )
    {
    case AbstractType::TypePointer:
    {
      PointerType::Ptr specificType = type.cast<PointerType>();
      addArgumentType( specificType->baseType() );
      break;
    }
    case AbstractType::TypeReference:
    {
      ReferenceType::Ptr specificType = type.cast<ReferenceType>();
      addArgumentType( specificType->baseType() );
      break;
    }
    case AbstractType::TypeArray:
    {
      ArrayType::Ptr specificType = type.cast<ArrayType>();
      addArgumentType( specificType->elementType() );
      break;
    }

    case AbstractType::TypeFunction:
    {
      //         FunctionType::Ptr specificType = type.cast<FunctionType>();
      //         addAssociatedFunction(specificType->declaration());
      //         break;
    }
    case AbstractType::TypeStructure:
    {
      StructureType::Ptr specificType = type.cast<StructureType>();
      addAssociatedClass( specificType->declaration( m_topContext.data() ) );
      break;
    }
    case AbstractType::TypeEnumeration:
    {
      EnumerationType::Ptr specificType = type.cast<EnumerationType>();
      addAssociatedNamespace( specificType->declaration( m_topContext.data() ) );
      break;
    }
    case AbstractType::TypeEnumerator:
    {
      EnumeratorType::Ptr specificType = type.cast<EnumeratorType>();
      addAssociatedNamespace( specificType->declaration( m_topContext.data() ) );
      break;
    }
    default:
      // the following types have empty associated namespaces lists
      /*      case AbstractType::TypeAbstract:
      case AbstractType::TypeAlias:
      case AbstractType::TypeIntegral:
      case AbstractType::TypeDelayed:
      case AbstractType::TypeUnsure:*/
      ;
    };
  };
}

QSet< Declaration* > ADLHelper::associatedNamespaces() const
{
  return m_associatedNamespaces;
}

void ADLHelper::addAssociatedClass( Declaration * declaration )
{
  if ( !declaration || !m_context || !m_topContext )
    return;

  /*
  From the standard:

  If T is a class type (including unions), its associated classes are: the class itself; the class of which it is a
  member, if any; and its direct and indirect base classes. Its associated namespaces are the namespaces
  in which its associated classes are defined.
  */

  // from the standard:
  // Typedef names and using-declarations used to specify the types do not contribute to this set.
  if ( declaration->isTypeAlias() || declaration->isAnonymous() )
    return;
  
  // for now just add the class namespace
  // TODO: implement the above policy
  DUContext* declContext = declaration->logicalInternalContext(m_topContext.data());
  if ( !declContext )
  {
    kDebug() << "declaration " << declaration->toString() << " has no logical internal context; skipping";
    return;
  }

  addAssociatedNamespace( declContext->scopeIdentifier() );
}

void ADLHelper::addAssociatedFunction( Declaration * declaration )
{
  if ( !declaration || ! m_context || ! m_topContext )
    return;

  // TODO: implement
}

void ADLHelper::addAssociatedNamespace( const QualifiedIdentifier & identifier )
{
  QList<Declaration*> decls = m_context->findDeclarations( identifier , KDevelop::SimpleCursor(), AbstractType::Ptr(), m_topContext.data() );
  foreach( Declaration * decl, decls )
  {
    addAssociatedNamespace( decl );
  }
}

void ADLHelper::addAssociatedNamespace( Declaration * declaration )
{
  if ( !declaration )
    return;

  if ( declaration->kind() == Declaration::Namespace ) {
    kDebug() << "adding namespace " << declaration->toString();
    m_associatedNamespaces += declaration;
  }
  // TODO: check if namespace aliases need to be resolved as well
  /*  else if (nsDeclaration.kind() == Declaration::NamespaceAlias) {
  addAssociatedNamespace(nsDecl);
  }*/
}


