/*
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "overloadresolution.h"
#include "cppduchain/typeutils.h"
#include <language/duchain/ducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/indexedstring.h>
#include <language/duchain/classfunctiondeclaration.h>
#include "cppduchain/cpptypes.h"
#include "viablefunctions.h"
#include "templatedeclaration.h"
#include "typeutils.h"
#include <QtAlgorithms>

using namespace Cpp;
using namespace KDevelop;

OverloadResolver::OverloadResolver( DUContextPointer context, TopDUContextPointer topContext  ) : m_context(context), m_topContext(topContext), m_worstConversionRank(NoMatch) {

}

Declaration* OverloadResolver::resolveConstructor( const ParameterList& params, bool implicit, bool noUserDefinedConversion ) {
    if( !m_context || !m_topContext )
      return 0;

    QList<Declaration*> goodDeclarations;
    Identifier id = m_context->localScopeIdentifier().last();
    id.clearTemplateIdentifiers();
    QList<Declaration*> declarations = m_context->findLocalDeclarations(id, KDevelop::SimpleCursor(), m_topContext.data(), AbstractType::Ptr(), DUContext::OnlyFunctions);

    for( QList<Declaration*>::iterator it = declarations.begin(); it != declarations.end(); ++it ) {
      if( (*it)->indexedType() )
      {
        FunctionType::Ptr function = (*it)->abstractType().cast<FunctionType>();
        ClassFunctionDeclaration* functionDeclaration = dynamic_cast<ClassFunctionDeclaration*>(*it);
        //Q_ASSERT();

        if( functionDeclaration /*&& functionDeclaration->isConstructor()*/ ) //Test not needed, because name == classname
        {
          if( function->indexedArgumentsSize() >= params.parameters.size() )
          {
            if( !implicit || !functionDeclaration->isExplicit() )
              goodDeclarations << *it;
          }
        }
      }
    }

    return resolveList( params, goodDeclarations, noUserDefinedConversion );
}

Declaration* OverloadResolver::resolve( const ParameterList& params, const QualifiedIdentifier& functionName, bool noUserDefinedConversion )
{
  if( !m_context || !m_topContext )
    return 0;

  QList<Declaration*> declarations = m_context->findDeclarations(functionName, KDevelop::SimpleCursor(), AbstractType::Ptr(), m_topContext.data());

  return resolveList(params, declarations, noUserDefinedConversion );
}

uint OverloadResolver::worstConversionRank() {
  return m_worstConversionRank;
}

void OverloadResolver::expandDeclarations( const QList<Declaration*>& declarations, QSet<Declaration*>& newDeclarations ) {
  for( QList<Declaration*>::const_iterator it = declarations.begin(); it != declarations.end(); ++it ) {
    Declaration* decl = *it;
    bool isConstant = false;

    if( CppClassType::Ptr klass = TypeUtils::realType(decl->abstractType(), m_topContext.data(), &isConstant).cast<CppClassType>() )
    {
      if( decl->kind() == Declaration::Instance ) {
        //Instances of classes should be substituted with their operator() members
        QList<Declaration*> decls;
        TypeUtils::getMemberFunctions( klass, m_topContext.data(), decls, "operator()", isConstant );

        foreach(Declaration* decl, decls)
          newDeclarations.insert(decl);
      } else {
        //Classes should be substituted with their constructors
        QList<Declaration*> decls;
        TypeUtils::getConstructors( klass, m_topContext.data(), decls );

        foreach(Declaration* decl, decls)
          newDeclarations.insert(decl);
      }
    }else{
      newDeclarations.insert(*it);
    }
  }
}

void OverloadResolver::expandDeclarations( const QList<QPair<OverloadResolver::ParameterList, Declaration*> >& declarations, QHash<Declaration*, OverloadResolver::ParameterList>& newDeclarations ) {
  for( QList<QPair<OverloadResolver::ParameterList, Declaration*> >::const_iterator it = declarations.begin(); it != declarations.end(); ++it ) {
    QPair<OverloadResolver::ParameterList, Declaration*> decl = *it;
    bool isConstant = false;

    if( CppClassType::Ptr klass = TypeUtils::realType(decl.second->abstractType(), m_topContext.data(), &isConstant).cast<CppClassType>() )
    {
      if( decl.second->kind() == Declaration::Instance ) {
        //Instances of classes should be substituted with their operator() members
        QList<Declaration*> functions;
        TypeUtils::getMemberFunctions( klass, m_topContext.data(), functions, "operator()", isConstant );
        foreach(Declaration* f, functions)
          newDeclarations.insert(f, decl.first);
      } else {
        //Classes should be substituted with their constructors
        QList<Declaration*> functions;
        TypeUtils::getConstructors( klass, m_topContext.data(), functions );
        foreach(Declaration* f, functions)
          newDeclarations.insert(f, decl.first);;
      }
    }else{
      newDeclarations.insert(it->second, it->first);
    }
  }
}

Declaration* OverloadResolver::resolveList( const ParameterList& params, const QList<Declaration*>& declarations, bool noUserDefinedConversion )
{
  if( !m_context || !m_topContext )
    return 0;

  ///Iso c++ draft 13.3.3
  m_worstConversionRank = ExactMatch;


  ///First step: Replace class-instances with operator() functions, and pure classes with their constructors
  QSet<Declaration*> newDeclarations;
  expandDeclarations( declarations, newDeclarations );

  ///Second step: Find best viable function
  ViableFunction bestViableFunction( m_topContext.data() );

  for( QSet<Declaration*>::const_iterator it = newDeclarations.begin(); it != newDeclarations.end(); ++it )
  {
    Declaration* decl = applyImplicitTemplateParameters( params, *it );
    if( !decl )
      continue;

    ViableFunction viable( m_topContext.data(), decl, noUserDefinedConversion );
    viable.matchParameters( params );

    if( viable.isBetter(bestViableFunction) ) {
      bestViableFunction = viable;
      m_worstConversionRank = bestViableFunction.worstConversion();
    }
  }

  if( bestViableFunction.isViable() )
    return bestViableFunction.declaration().data();
  else
    return 0;
}

QList< ViableFunction > OverloadResolver::resolveListOffsetted( const ParameterList& params, const QList<QPair<OverloadResolver::ParameterList, Declaration*> >& declarations, bool partial  )
{
  if( !m_context || !m_topContext )
    return QList<ViableFunction>();

  ///Iso c++ draft 13.3.3
  m_worstConversionRank = ExactMatch;

  ///First step: Replace class-instances with operator() functions, and pure classes with their constructors
  QHash<Declaration*, OverloadResolver::ParameterList> newDeclarations;
  expandDeclarations( declarations, newDeclarations );

  ///Second step: Find best viable function
  QList<ViableFunction> viableFunctions;

  for( QHash<Declaration*, OverloadResolver::ParameterList>::const_iterator it = newDeclarations.begin(); it != newDeclarations.end(); ++it )
  {
    ViableFunction viable( m_topContext.data(), it.key() );
    ParameterList mergedParams = it.value();
    mergedParams.parameters += params.parameters;
    viable.matchParameters( mergedParams, partial );

    viableFunctions << viable;
  }

  qSort( viableFunctions );

  return viableFunctions;
}

Declaration* OverloadResolver::applyImplicitTemplateParameters( const ParameterList& params, Declaration* declaration ) const
{
  TemplateDeclaration* tempDecl = dynamic_cast<TemplateDeclaration*>(declaration);
  if(!tempDecl)
    return declaration;

  KDevelop::DUContext* templateContext = tempDecl->templateParameterContext();
  if(!templateContext) {
    //May be just within a template, but without own template parameters
    return declaration;
  }

  FunctionType::Ptr functionType = declaration->type<FunctionType>();
  if(!functionType) {
    kDebug(9007) << "Template function has no function type";
    return declaration;
  }

  const IndexedType* arguments( functionType->indexedArguments() );
  if(params.parameters.count() > functionType->indexedArgumentsSize())
    return declaration;

  //templateContext contains the template-parameters that we need to find instantiations for

  QMap<IndexedString, AbstractType::Ptr> instantiatedParameters; //Here we store the values assigned to each template-parameter

  foreach( Declaration* decl, templateContext->localDeclarations() ) {
    CppTemplateParameterType::Ptr paramType = decl->abstractType().cast<CppTemplateParameterType>();
    if( paramType ) { //Parameters that are not of type CppTemplateParameterType are already assigned.
      instantiatedParameters[decl->identifier().identifier()] = AbstractType::Ptr();
    }
  }

  if( instantiatedParameters.isEmpty() )
    return declaration; //All parameters already have a type assigned

  for( int a = 0; a < params.parameters.count(); a++ )
    matchParameterTypes(params.parameters[a].type, arguments[a].type(), instantiatedParameters);

  bool allInstantiated = true;
  for( QMap<IndexedString, AbstractType::Ptr>::const_iterator it = instantiatedParameters.begin(); it != instantiatedParameters.end(); ++it )
    if( !(*it) ) {
      allInstantiated = false;
    }

  if( allInstantiated ) {
    //We have new template-parameters at hand, we can specialize now.
    Cpp::InstantiationInformation instantiateWith(tempDecl->instantiatedWith().information());
    instantiateWith.templateParametersList().clear();

    foreach( Declaration* decl, templateContext->localDeclarations() ) {
      IndexedType type;

      CppTemplateParameterType::Ptr paramType = decl->abstractType().cast<CppTemplateParameterType>();
      if( paramType ) //Take the type we have assigned.
        type = instantiatedParameters[decl->identifier().identifier()]->indexed();
      else
        type = decl->abstractType()->indexed(); //Take the type that was available already earlier

      instantiateWith.templateParametersList().append(type);
    }
    Declaration* ret = tempDecl->instantiate( instantiateWith, m_topContext.data() );
    ///@todo find out when can fail
    //Q_ASSERT(ret->id().getDeclaration(m_topContext.data()) == ret);
    return ret;
  }

  return declaration;
}

uint OverloadResolver::matchParameterTypes(const AbstractType::Ptr& argumentType, const AbstractType::Ptr& parameterType, QMap<IndexedString, AbstractType::Ptr>& instantiatedTypes) const
{
//    kDebug() << "matching" << argumentType->toString() << "to" << parameterType->toString();

  if(!argumentType && !parameterType)
    return 1;
  if(!argumentType || !parameterType)
    return 0;
  if(instantiatedTypes.isEmpty())
    return 1;

  DelayedType::Ptr delayed = parameterType.cast<DelayedType>();
  if(delayed)
    return 1 + matchParameterTypes( argumentType, delayed->identifier(), instantiatedTypes );

  ///In case of references on both sides, match the target-types
  ReferenceType::Ptr argumentRef = argumentType.cast<ReferenceType>();
  ReferenceType::Ptr parameterRef = parameterType.cast<ReferenceType>();

  if( argumentRef && parameterRef )
    return 1 + matchParameterTypes( argumentRef->baseType(), parameterRef->baseType(), instantiatedTypes );
  else if(argumentRef)
    return 1 + matchParameterTypes( argumentRef->baseType(), parameterType, instantiatedTypes );
  else if(parameterRef)
    return 1 + matchParameterTypes( argumentType, parameterRef->baseType(), instantiatedTypes );

  ///In case of pointers on both sides, match the target-types
  PointerType::Ptr argumentPointer = argumentType.cast<PointerType>();
  PointerType::Ptr parameterPointer = parameterType.cast<PointerType>();

  if( argumentPointer && parameterPointer )
    return 1 + matchParameterTypes( argumentPointer->baseType(), parameterPointer->baseType(), instantiatedTypes );

  ///Match assigned template-parameters, for example when matching QList<int> to QList<T>, assign int to T.
  const IdentifiedType* identifiedArgument = dynamic_cast<const IdentifiedType*>(argumentType.unsafeData());
  const IdentifiedType* identifiedParameter = dynamic_cast<const IdentifiedType*>(parameterType.unsafeData());

  uint matchDepth = 1;
  
  if( identifiedArgument && identifiedParameter )
  {
    ///@todo 1. Don't use qualifiedIdentifier(), and 2. think about this, it seems not correct
    if( identifiedArgument->qualifiedIdentifier() == identifiedParameter->qualifiedIdentifier() )
    {
      TemplateDeclaration* argumentTemplateDeclaration = dynamic_cast<TemplateDeclaration*>(identifiedArgument->declaration(m_topContext.data()));
      TemplateDeclaration* parameterTemplateDeclaration = dynamic_cast<TemplateDeclaration*>(identifiedParameter->declaration(m_topContext.data()));

      if( argumentTemplateDeclaration && parameterTemplateDeclaration )
      {
        DUContext* argumentTemplateDeclarationContext = argumentTemplateDeclaration->templateParameterContext();
        DUContext* parameterTemplateDeclarationContext = parameterTemplateDeclaration->templateParameterContext();
        if(parameterTemplateDeclarationContext && argumentTemplateDeclarationContext) {
            if( argumentTemplateDeclarationContext->localDeclarations().count() == parameterTemplateDeclarationContext->localDeclarations().count() ) {
            for( int a = 0; a < argumentTemplateDeclarationContext->localDeclarations().count(); ++a )
                matchDepth += matchParameterTypes( argumentTemplateDeclarationContext->localDeclarations()[a]->abstractType(), parameterTemplateDeclarationContext->localDeclarations()[a]->abstractType(), instantiatedTypes );
            }
        }else{
          kDebug() << "missing template argument context";
        }
      }
    }
  }

  return matchDepth;
}

AbstractType::Ptr getContainerType(AbstractType::Ptr type, int depth, TopDUContext* topContext) {
  for(int a = 0; a < depth; a++) {
    AbstractType::Ptr real = TypeUtils::realType(type, topContext);
    IdentifiedType* idType = dynamic_cast<IdentifiedType*>(real.unsafeData());
    if(!idType)
      return AbstractType::Ptr();

    Declaration* decl = idType->declaration(topContext);
    Declaration* containerDecl = decl->context()->owner();

    if(containerDecl)
      type = containerDecl->abstractType();
    else
      return AbstractType::Ptr();
  }
  return type;
}

uint OverloadResolver::matchParameterTypes(AbstractType::Ptr argumentType, const TypeIdentifier& parameterType, QMap<IndexedString, AbstractType::Ptr>& instantiatedTypes) const
{
//    kDebug() << "1 matching" << argumentType->toString() << "to" << parameterType.toString();
  if(!argumentType)
    return 1;
  if(instantiatedTypes.isEmpty())
    return 1;
  if(parameterType.isEmpty())
    return 1;

  {
    ReferenceType::Ptr argumentRef = argumentType.cast<ReferenceType>();

    if( argumentRef && parameterType.isReference() )
      argumentType = argumentRef->baseType();
    else if( parameterType.isReference() )
      return 0; //Reference on right side, but not on left
  }
  {
    PointerType::Ptr argumentPointer = argumentType.cast<PointerType>();
    int cnt = 0; ///@todo correct ordering of the pointers and their constnesses
    while( argumentPointer && cnt < parameterType.pointerDepth() ) {
      ++cnt;
      argumentType = argumentPointer->baseType();
      argumentPointer = argumentType.cast<PointerType>();
    }
    if( cnt != parameterType.pointerDepth() || !argumentType )
      return 0; //Do not have the needed count of pointers
  }

  uint matchDepth = 1;

  for( int a = 0; a < parameterType.count(); ++a ) {
    AbstractType::Ptr pType = getContainerType(argumentType, parameterType.count() - a - 1, m_topContext.data());
    uint localDepth = matchParameterTypes(pType, parameterType.at(a), instantiatedTypes);
    if(!localDepth)
      return 0;
    matchDepth += localDepth;
  }
  return matchDepth;
}

uint OverloadResolver::matchParameterTypes(AbstractType::Ptr argumentType, const Identifier& parameterType, QMap<IndexedString, AbstractType::Ptr>& instantiatedTypes) const
{
//    kDebug() << "2 matching" << argumentType->toString() << "to" << parameterType.toString();

  if(!argumentType)
    return 1;
  if(instantiatedTypes.isEmpty())
    return 1;


  if( instantiatedTypes.contains(parameterType.identifier()) ) {
    if(ConstantIntegralType::Ptr integral = argumentType.cast<ConstantIntegralType>())
      //Extract the actual type without the value
      argumentType = AbstractType::Ptr(new IntegralType(*integral));
    
    instantiatedTypes[parameterType.identifier()] = argumentType;
    return 1;
  }

  IdentifiedType* identifiedArgument = dynamic_cast<IdentifiedType*>(argumentType.unsafeData());
  if(!identifiedArgument)
    return 0;
  
  if( identifiedArgument->qualifiedIdentifier().last().identifier() != parameterType.identifier() ) {
    return 0;
  }

  TemplateDeclaration* argumentTemplateDeclaration = dynamic_cast<TemplateDeclaration*>(identifiedArgument->declaration(m_topContext.data()));

  uint matchDepth = 1;
  
  if( argumentTemplateDeclaration && parameterType.templateIdentifiersCount() )
  {
    DUContext* argumentTemplateDeclarationContext = argumentTemplateDeclaration->templateParameterContext();

    if( argumentTemplateDeclarationContext )
    {
      int matchLength = argumentTemplateDeclarationContext->localDeclarations().count();
      if( parameterType.templateIdentifiersCount() < matchLength )
        matchLength = parameterType.templateIdentifiersCount();

      for( int a = 0; a < matchLength; ++a )
        matchDepth += matchParameterTypes( argumentTemplateDeclarationContext->localDeclarations()[a]->abstractType(), parameterType.templateIdentifier(a), instantiatedTypes );
    }else{
       kDebug(9007) << "Template-declaration missing template-parameter context";
    }
  }

  return matchDepth;
}

///Version of matchParameters that would be correct if we used CppTemplateParameterType* instead of DelayedType. Maybe we should doing that some time, it's more correct, and more complicated.
// bool OverloadResolver::matchParameterTypes(AbstractType* _argumentType, AbstractType* _parameterType, QMap<CppTemplateParameterType*, AbstractType::Ptr>& instantiatedTypes) const
// {
//   if(!_argumentType || !_parameterType)
//     return true;
//   if(instantiatedTypes.isEmpty())
//     return true;
//
//   AbstractType::Ptr argumentType = TypeUtils::resolvedType(_argumentType, m_topContext.data());
//   AbstractType::Ptr parameterType = TypeUtils::resolvedType(_parameterType, m_topContext.data());
//
//   CppTemplateParameterType* temp = dynamic_cast<CppTemplateParameterType*>(parameterType.data());
//   if(temp) {
//     if( instantiatedTypes.contains(temp) ) {
//       if( instantiatedTypes[temp].data() == 0 ) {
//         instantiatedTypes[temp] = argumentType;
//       }else{
//         ///@todo compare if the type already instantiated for this parameter is the same as argumentType, and else fail.
//       }
//     }
//     return true;
//   }
//
//   ///Match assigned template-parameters, for example when matching QList<int> to QList<T>, assign int to T.
//   IdentifiedType* identifiedArgument = dynamic_cast<IdentifiedType*>(argumentType.data());
//   IdentifiedType* identifiedParameter = dynamic_cast<IdentifiedType*>(parameterType.data());
//
//   if( identifiedArgument && identifiedParameter )
//   {
//     if( identifiedArgument->identifier() == identifiedParameter->identifier() )
//     {
//       TemplateDeclaration* argumentTemplateDeclaration = dynamic_cast<TemplateDeclaration*>(identifiedArgument->declaration());
//       TemplateDeclaration* parameterTemplateDeclaration = dynamic_cast<TemplateDeclaration*>(identifiedParameter->declaration());
//
//       if( argumentTemplateDeclaration && parameterTemplateDeclaration )
//       {
//         DUContext* argumentTemplateDeclarationContext = argumentTemplateDeclaration->templateParameterContext();
//         DUContext* parameterTemplateDeclarationContext = parameterTemplateDeclaration->templateParameterContext();
//
//         if( argumentTemplateDeclarationContext->localDeclarations().count() == parameterTemplateDeclarationContext->localDeclarations().count() ) {
//           for( int a = 0; a < argumentTemplateDeclarationContext->localDeclarations().count(); ++a )
//             matchParameterTypes( argumentTemplateDeclarationContext->localDeclarations()[a]->abstractType().data(), parameterTemplateDeclarationContext->localDeclarations()[a]->abstractType().data(), instantiatedTypes );
//         }
//       }
//     }
//   }
//
//   ///In case of references on both sides, match the target-types
//   ReferenceType* argumentRef = dynamic_cast<ReferenceType*>(argumentType.data());
//   ReferenceType* parameterRef = dynamic_cast<ReferenceType*>(parameterType.data());
//
//   if( argumentRef && parameterRef )
//     return matchParameterTypes( argumentRef->baseType().data(), parameterRef->baseType().data(), instantiatedTypes );
//
//   ///In case of pointers on both sides, match the target-types
//   PointerType* argumentPointer = dynamic_cast<PointerType*>(argumentType.data());
//   PointerType* parameterPointer = dynamic_cast<PointerType*>(parameterType.data());
//
//   if( argumentPointer && parameterPointer )
//     return matchParameterTypes( argumentPointer->baseType().data(), parameterPointer->baseType().data(), instantiatedTypes );
//
//   return true;
// }
