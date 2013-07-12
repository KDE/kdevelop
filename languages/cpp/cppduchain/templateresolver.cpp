/*
   Copyright 2012 Olivier de Gaalon <olviier.jg@gmail.com>

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

#include "templateresolver.h"
#include "cpptypes.h"
#include "templatedeclaration.h"
#include "templateparameterdeclaration.h"

using namespace Cpp;
using namespace KDevelop;

TemplateResolver::TemplateResolver(const TopDUContext* topContext)
:m_topContext(topContext) { }

uint TemplateResolver::matchTemplateParameterTypes( const AbstractType::Ptr& argumentType, const AbstractType::Ptr& parameterType, QMap< IndexedString, AbstractType::Ptr >& instantiatedTypes ) const
{
  if ( !argumentType && !parameterType )
    return 1;
  if ( !argumentType || !parameterType )
    return 0;

  TemplateMatchType matchResult;
  matchTemplateParameterTypesInternal(argumentType, parameterType, instantiatedTypes, matchResult);
  return matchResult.toUint();
}

bool isConstBased(AbstractType::Ptr type)
{
  if (type->modifiers() & AbstractType::ConstModifier)
    return true;
  if (ArrayType::Ptr arrayType = type.cast<ArrayType>())
    return arrayType->elementType() ? isConstBased(arrayType->elementType()) : false;
  if (PointerType::Ptr ptrType = type.cast<PointerType>())
    return ptrType->baseType() ? isConstBased(ptrType->baseType()) : false;
  return false;
}

bool TemplateResolver::templateHandleConstIntegralType(const AbstractType::Ptr& argumentType, const AbstractType::Ptr& parameterType, TemplateMatchType& res) const
{
  ConstantIntegralType::Ptr argumentIntegral = argumentType.cast<ConstantIntegralType>();
  ConstantIntegralType::Ptr parameterIntegral = parameterType.cast<ConstantIntegralType>();
  if (argumentIntegral && parameterIntegral)
  {
    if (argumentIntegral->plainValue() != parameterIntegral->plainValue())
      res.valid = false;
    return true; //Handled, valid if integral types match, invalid otherwise
  }
  else if (parameterIntegral)
  {
    //Nothing but an equal integral will match an integral parameter
    res.valid = false;
    return true; //Handled, invalid.
  }
  else if (argumentIntegral && !parameterType.cast<DelayedType>())
  {
    res.valid = false; //A const integral arg can only match an equal const integral or replace a delayed type
    return true; //Handled, invalid
  }
  return false;
}

bool TemplateResolver::templateHandleDelayedType ( const AbstractType::Ptr& argumentType, const AbstractType::Ptr& parameterType, QMap< IndexedString, AbstractType::Ptr >& instantiatedTypes, TemplateMatchType& res ) const
{
  DelayedType::Ptr delayed = parameterType.cast<DelayedType>();
  if ( !delayed )
    return false; //Not delayed type, not handled

  IndexedTypeIdentifier paramDelayedId = delayed->identifier();
  //Delayed id should never have pointer depth, or it would be a pointerType and not a delayedType
  //If it's possible somehow for it to be both, it's not correctly handled here
  Q_ASSERT(!paramDelayedId.pointerDepth());
  if ( paramDelayedId.isConstant() )
  {
    if ( isConstBased(argumentType) && !argumentType.cast<PointerType>() )
      res.constMatch = true;
    else
    {
      //Case 1: Parameter is const, argument is not, no match
      //Case 2: Argument is a "const type" with at least one pointer depth,
      //  parameter type is a "const type" with no further pointer depth, no match
      res.valid = false;
    }
  }
  ///TODO: the code only uses the last identifier and used to verify that
  ///      only one Identifier is actually contained in the QualifiedIdentifier
  ///      in the paramDelayedId
  ///      This caused issues for __gnu_cxx::_S_mutex, _S_single, _S_atomic etc.
  ///      it's not clear whether this is actually a bug or not - someone should
  ///      investigate. But rather don't assert for now!
  IndexedString identifier = paramDelayedId.identifier().identifier().last().identifier();
  if ( instantiatedTypes.contains( identifier ) )
    instantiatedTypes[identifier] = argumentType;
  else
    res.valid = false;

  return true; //Parameter was delayed type, delayed type handled
}

bool TemplateResolver::templateHandleReferenceType(const AbstractType::Ptr& argumentType, const AbstractType::Ptr& parameterType, QMap< IndexedString, AbstractType::Ptr >& instantiatedTypes, TemplateMatchType& res) const
{
  ReferenceType::Ptr argumentRef = argumentType.cast<ReferenceType>();
  ReferenceType::Ptr parameterRef = parameterType.cast<ReferenceType>();
  if ( argumentRef && parameterRef && argumentRef->isRValue() == parameterRef->isRValue() )
  {
    ///In case of references on both sides, match the target-types
    res.referenceMatch = true;
    matchTemplateParameterTypesInternal( argumentRef->baseType(), parameterRef->baseType(), instantiatedTypes, res );
    return true; //Handled by matching base types
  }
  else if (argumentRef)
  {
    //Argument is a reference of some sort, but will match non-reference CppTemplateParameterType
    if (parameterType.cast<CppTemplateParameterType>())
      matchTemplateParameterTypesInternal( argumentRef->baseType(), parameterType, instantiatedTypes, res);
    else
      res.valid = false;
    return true; //Handled by matching argument base type against template param,
                 //or invalidated because argument is ref and param isn't
  }
  else if ( parameterRef )
  {
    res.valid = false;
    return true; //Handled, invalid as the parameter is a reference type but the argument is not
  }

  return false; //No references, not handled
}

bool TemplateResolver::templateHandlePointerType(const AbstractType::Ptr& argumentType, const AbstractType::Ptr& parameterType, QMap< IndexedString, AbstractType::Ptr >& instantiatedTypes, TemplateMatchType& res) const
{
  ///In case of pointers on both sides, match the target-types
  PointerType::Ptr argumentPointer = argumentType.cast<PointerType>();
  PointerType::Ptr parameterPointer = parameterType.cast<PointerType>();
  if ( argumentPointer && parameterPointer && (( argumentPointer->modifiers() & AbstractType::ConstModifier ) == ( parameterPointer->modifiers() & AbstractType::ConstModifier ) ) )
  {
    ++res.pointerMatchDepth;
    matchTemplateParameterTypesInternal( argumentPointer->baseType(), parameterPointer->baseType(), instantiatedTypes, res );
    return true; //Handled by matching base types
  }
  else if (argumentPointer)
  {
    if (!parameterPointer && !isConstBased(parameterType) && parameterType.cast<CppTemplateParameterType>())
    {
        matchTemplateParameterTypesInternal( argumentPointer->baseType(), parameterType, instantiatedTypes, res );
        return true; //Handled by matching argument base type
    }
    //If argument is a pointer (const or otherwise), it will not match a non-pointer const parameter
    //Even if parameter isn't const, unless it's a CppTemplateParameterType it won't match
    res.valid = false;
    return true; //Handled, invalid
  }
  else if (parameterPointer)
  {
    res.valid = false;
    return true; //Handled, invalid as parameter had addition unmatched pointer depth
  }

  return false; //Not handled, neither argument nor parameter are pointers
}

bool TemplateResolver::templateHandleArrayType(const AbstractType::Ptr& argumentType, const AbstractType::Ptr& parameterType, QMap< IndexedString, AbstractType::Ptr >& instantiatedTypes, TemplateMatchType& res) const
{
  ArrayType::Ptr argumentArray = argumentType.cast<ArrayType>();
  ArrayType::Ptr parameterArray = parameterType.cast<ArrayType>();
  if ( argumentArray && parameterArray && (( argumentArray->modifiers() & AbstractType::ConstModifier ) == ( parameterArray->modifiers() & AbstractType::ConstModifier ) ) ) {
    if ( argumentArray->modifiers() & AbstractType::ConstModifier )
      res.constMatch = true;
    res.arrayMatch = true;
    matchTemplateParameterTypesInternal( argumentArray->elementType(), parameterArray->elementType(), instantiatedTypes, res );
    return true;
  }
  else if (argumentArray)
  {
    //Argument type is array, but will match non-array CppTemplateParameterType
    if (parameterType.cast<CppTemplateParameterType>())
      matchTemplateParameterTypesInternal( argumentArray->elementType(), parameterType, instantiatedTypes, res );
    else
      res.valid = false;
    return true; //Handled, either by matching argument elementType or by invalidation because parameter type cannot match
  }
  else if (parameterArray)
  {
    res.valid = false; //Parameter is array type, argument must be array type
    return true;
  }
  return false;
}

bool TemplateResolver::templateHandleIdentifiedType(const AbstractType::Ptr& argumentType, const AbstractType::Ptr& parameterType, QMap< IndexedString, AbstractType::Ptr >& instantiatedTypes, TemplateMatchType& res) const
{
  ///Match assigned template-parameters, for example when matching QList<int> to QList<T>, assign int to T.
  const IdentifiedType* identifiedArgument = dynamic_cast<const IdentifiedType*>( argumentType.unsafeData() );
  const IdentifiedType* identifiedParameter = dynamic_cast<const IdentifiedType*>( parameterType.unsafeData() );

  if ( identifiedArgument && identifiedParameter )
  {
    Declaration* argumentDeclaration = identifiedArgument->declaration( m_topContext );
    Declaration* parameterDeclaration = identifiedParameter->declaration( m_topContext );
    if (!argumentDeclaration || !parameterDeclaration)
    {
      //TODO: Very rare case which may be a bug elsewhere and could use a test.
      return false; //Unhandled, let it be accepted or rejected later
    }
    TemplateDeclaration* argumentTemplateDeclaration = dynamic_cast<TemplateDeclaration*>( argumentDeclaration );
    TemplateDeclaration* parameterTemplateDeclaration = dynamic_cast<TemplateDeclaration*>( parameterDeclaration );
    if ( !argumentTemplateDeclaration || !parameterTemplateDeclaration )
    {
      //Is this as correct as using the indexedType of the non-template declarations?
      if (argumentDeclaration != parameterDeclaration)
        res.valid = false; //Two different non-template declarations == two different types
      return true; //Handled, with either invalid mismatched types or valid matched types
    }

    if ( argumentTemplateDeclaration->instantiatedFrom() == parameterTemplateDeclaration->instantiatedFrom() && argumentTemplateDeclaration->instantiatedFrom() )
    {
      InstantiationInformation argumentInstantiatedWith = argumentTemplateDeclaration->instantiatedWith().information();
      InstantiationInformation parameterInstantiatedWith = parameterTemplateDeclaration->instantiatedWith().information();
      if ( argumentInstantiatedWith.templateParametersSize() != parameterInstantiatedWith.templateParametersSize() )
      {
        res.valid = false;
        return true; //Handled, invalid
      }

      for ( uint a = 0; a < argumentInstantiatedWith.templateParametersSize(); ++a )
      {
        if ( !matchTemplateParameterTypes( argumentInstantiatedWith.templateParameters()[a].abstractType(), parameterInstantiatedWith.templateParameters()[a].abstractType(), instantiatedTypes) )
        {
          res.valid = false;
          return true; //Handled, invalid
        }
      }
      res.templateArgsMatch = true;
      return true; //Handled, valid
    }
  }
  else if (identifiedArgument || identifiedParameter)
  {
    //Is there any case wherein an identifiedArgument will go up against a CppTemplateParameterType?
    //If This is possible we need a test case
    Q_ASSERT(!parameterType.cast<CppTemplateParameterType>());
    res.valid = false;
    return true; //Handled, invalid. If only one is identified, it's not a match
  }

  return false;
}

void TemplateResolver::matchTemplateParameterTypesInternal ( const AbstractType::Ptr& argumentType, const AbstractType::Ptr& parameterType, QMap< IndexedString, AbstractType::Ptr >& instantiatedTypes, TemplateMatchType& res ) const
{
  if (!argumentType || !parameterType)
  {
    kWarning() << "Invalid Type Encountered";
    res.valid = false;
    return;
  }

  if (templateHandleConstIntegralType(argumentType, parameterType, res))
    return;
  if (templateHandleDelayedType(argumentType, parameterType, instantiatedTypes, res))
    return;
  if (templateHandleReferenceType(argumentType, parameterType, instantiatedTypes, res))
    return;
  if (templateHandlePointerType(argumentType, parameterType, instantiatedTypes, res))
    return;
  if (templateHandleArrayType(argumentType, parameterType, instantiatedTypes, res))
    return;

  if (isConstBased(parameterType))
  {
    if (!argumentType.cast<PointerType>() && isConstBased(argumentType))
      res.constMatch = true;
    else
    {
      res.valid = false;
      return; //Invalid, param is const and arg is either non-const or has a different ptr-depth
    }
  }

  if ( CppTemplateParameterType::Ptr templateParam = parameterType.cast<CppTemplateParameterType>() )
  {
    Declaration* decl = templateParam->declaration( m_topContext );
    if ( decl )
    {
      //Should not be possible to have a CPPTemplateParameterType with template ids..?
      Q_ASSERT(decl->identifier().templateIdentifiersCount() == 0);
      IndexedString id = decl->identifier().identifier();
      //FIXME: Sometimes when matching templates within templates, delayedType will set the identifier first
      //The other way around is also probably possible
      //This needs more work to make sure the right type is set here
      //Q_ASSERT(instantiatedTypes[id].isNull());
      instantiatedTypes[id] = argumentType;
      return;
    }
  }

  if (templateHandleIdentifiedType(argumentType, parameterType, instantiatedTypes, res))
    return;

  //This /should/ be correct for all unhandled cases
  if (parameterType->indexed() != argumentType->indexed())
    res.valid = false; //Invalid, types don't match
}
