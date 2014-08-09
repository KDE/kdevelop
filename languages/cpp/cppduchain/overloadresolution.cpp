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
#include <serialization/indexedstring.h>
#include <language/duchain/classfunctiondeclaration.h>
#include "cppduchain/cpptypes.h"
#include "viablefunctions.h"
#include "templatedeclaration.h"
#include "templateparameterdeclaration.h"
#include "typeutils.h"
#include <QtAlgorithms>
#include "adlhelper.h"
#include "typeconversion.h"
#include <language/duchain/persistentsymboltable.h>

using namespace Cpp;
using namespace KDevelop;

const bool allowADL = true;

// uncomment to get debugging info on ADL - very expensive on parsing
// #define DEBUG_ADL

#define ifDebugOverloadResolution(x)
// #define ifDebugOverloadResolution(x) x

OverloadResolver::OverloadResolver( DUContextPointer context, TopDUContextPointer topContext, Constness constness, bool forceIsInstance )
: m_context( context )
, m_topContext( topContext )
, m_worstConversionRank( NoMatch )
, m_forceIsInstance( forceIsInstance )
, m_constness(constness)
{

}

Declaration* OverloadResolver::resolveConstructor( const ParameterList& params, bool implicit, bool noUserDefinedConversion )
{
  if ( !m_context || !m_topContext )
    return 0;

  QList<Declaration*> goodDeclarations;
  Identifier id = m_context->localScopeIdentifier().last();
  id.clearTemplateIdentifiers();
  QList<Declaration*> declarations = m_context->findLocalDeclarations( id, CursorInRevision::invalid(), m_topContext.data(), AbstractType::Ptr(), DUContext::OnlyFunctions );

  for ( QList<Declaration*>::iterator it = declarations.begin(); it != declarations.end(); ++it )
  {
    if (( *it )->indexedType() )
    {
      FunctionType::Ptr function = ( *it )->abstractType().cast<FunctionType>();
      ClassFunctionDeclaration* functionDeclaration = dynamic_cast<ClassFunctionDeclaration*>( *it );
      //Q_ASSERT();

      if ( functionDeclaration /*&& functionDeclaration->isConstructor()*/ ) //Test not needed, because name == classname
      {
        if ( function->indexedArgumentsSize() >= static_cast<uint>(params.parameters.size()) )
        {
          if ( !implicit || !functionDeclaration->isExplicit() )
            goodDeclarations << *it;
        }
      }
    }
  }

  // no ADL possible when resolving constructors
  return resolveList( params, goodDeclarations, noUserDefinedConversion );
}

Declaration* OverloadResolver::resolve( const ParameterList& params, const QualifiedIdentifier& functionName, bool noUserDefinedConversion )
{
  if ( !m_context || !m_topContext )
    return 0;

  QList<Declaration*> declarations = m_context->findDeclarations( functionName, CursorInRevision::invalid(), AbstractType::Ptr(), m_topContext.data() );

  // without ADL findDeclarations may fail so skip ADL there and do it here
  Declaration * resolvedDecl = resolveList( params, declarations, noUserDefinedConversion );

  if (!resolvedDecl && functionName.count() == 1) {
    // start ADL lookup
    
    QList<Declaration*> adlDecls = computeADLCandidates( params, functionName );
    
    resolvedDecl = resolveList( params, adlDecls, noUserDefinedConversion );
#ifdef DEBUG_ADL
    if (resolvedDecl)
      kDebug() << "ADL found " << resolvedDecl->toString();
    else
      kDebug() << "ADL failed";
#endif
  }
  return resolvedDecl;
}

uint OverloadResolver::worstConversionRank()
{
  return m_worstConversionRank;
}

void OverloadResolver::expandDeclarations( const QList<Declaration*>& declarations, QSet<Declaration*>& newDeclarations )
{
  for ( QList<Declaration*>::const_iterator it = declarations.constBegin(); it != declarations.constEnd(); ++it )
  {
    Declaration* decl = *it;

    if ( CppClassType::Ptr klass = TypeUtils::realType( decl->abstractType(), m_topContext.data() ).cast<CppClassType>() )
    {
      if ( decl->kind() == Declaration::Instance || m_forceIsInstance )
      {
        //Instances of classes should be substituted with their operator() members
        QList<Declaration*> decls;
        TypeUtils::getMemberFunctions( klass, m_topContext.data(), decls, "operator()", klass->modifiers() & AbstractType::ConstModifier );

        foreach( Declaration* decl, decls )
        newDeclarations.insert( decl );
      }
      else
      {
        //Classes should be substituted with their constructors
        foreach( Declaration* decl, TypeUtils::getConstructors( klass, m_topContext.data() ) ) {
          newDeclarations.insert( decl );
        }
      }
    }
    else
    {
      newDeclarations.insert( *it );
    }
  }
}

void OverloadResolver::expandDeclarations( const QList<QPair<OverloadResolver::ParameterList, Declaration*> >& declarations, QHash<Declaration*, OverloadResolver::ParameterList>& newDeclarations )
{
  for ( QList<QPair<OverloadResolver::ParameterList, Declaration*> >::const_iterator it = declarations.constBegin(); it != declarations.constEnd(); ++it )
  {
    QPair<OverloadResolver::ParameterList, Declaration*> decl = *it;

    if ( CppClassType::Ptr klass = TypeUtils::realType( decl.second->abstractType(), m_topContext.data() ).cast<CppClassType>() )
    {
      if ( decl.second->kind() == Declaration::Instance || m_forceIsInstance )
      {
        //Instances of classes should be substituted with their operator() members
        QList<Declaration*> functions;
        TypeUtils::getMemberFunctions( klass, m_topContext.data(), functions, "operator()", klass->modifiers() & AbstractType::ConstModifier );
        foreach( Declaration* f, functions )
        newDeclarations.insert( f, decl.first );
      }
      else
      {
        //Classes should be substituted with their constructors
        foreach( Declaration* f, TypeUtils::getConstructors( klass, m_topContext.data() ) ) {
          newDeclarations.insert( f, decl.first );
        }
      }
    }
    else
    {
      newDeclarations.insert( it->second, it->first );
    }
  }
}

Declaration* OverloadResolver::resolveList( const ParameterList& params, const QList<Declaration*>& declarations, bool noUserDefinedConversion )
{
  if ( !m_context || !m_topContext )
    return 0;

  ///Iso c++ draft 13.3.3
  m_worstConversionRank = ExactMatch;


  ///First step: Replace class-instances with operator() functions, and pure classes with their constructors
  QSet<Declaration*> newDeclarations;
  expandDeclarations( declarations, newDeclarations );

  ///Second step: Find best viable function
  ViableFunction bestViableFunction( m_topContext.data() );

  for ( QSet<Declaration*>::const_iterator it = newDeclarations.constBegin(); it != newDeclarations.constEnd(); ++it )
  {
    Declaration* decl = applyImplicitTemplateParameters( params, *it );
    ifDebugOverloadResolution(qDebug() << (*it)->toString() << decl; )
    if ( !decl )
      continue;

    ViableFunction viable( m_topContext.data(), decl, m_constness, noUserDefinedConversion );
    viable.matchParameters( params );

    ifDebugOverloadResolution(qDebug() << decl->toString() << viable.isBetter(bestViableFunction); )
    if ( viable.isBetter( bestViableFunction ) )
    {
      bestViableFunction = viable;
      m_worstConversionRank = bestViableFunction.worstConversion();
    }
  }

  if ( bestViableFunction.isViable() )
    return bestViableFunction.declaration().data();
  else {
    return 0;
  }
}

QList< ViableFunction > OverloadResolver::resolveListOffsetted( const ParameterList& params, const QList<QPair<OverloadResolver::ParameterList, Declaration*> >& declarations, bool partial )
{
  if ( !m_context || !m_topContext )
    return QList<ViableFunction>();

  ///Iso c++ draft 13.3.3
  m_worstConversionRank = ExactMatch;

  ///First step: Replace class-instances with operator() functions, and pure classes with their constructors
  QHash<Declaration*, OverloadResolver::ParameterList> newDeclarations;
  expandDeclarations( declarations, newDeclarations );

  ///Second step: Find best viable function
  QList<ViableFunction> viableFunctions;

  for ( QHash<Declaration*, OverloadResolver::ParameterList>::const_iterator it = newDeclarations.constBegin(); it != newDeclarations.constEnd(); ++it )
  {
    ParameterList mergedParams = it.value();
    mergedParams.parameters += params.parameters;
    
    Declaration* decl = applyImplicitTemplateParameters( mergedParams, it.key() );
    ifDebugOverloadResolution(qDebug() << it.key()->toString() << decl; )
    if ( !decl )
      continue;

    if (decl->isExplicitlyDeleted())
      continue;

    ViableFunction viable( m_topContext.data(), decl, m_constness );
    viable.matchParameters( mergedParams, partial );

    viableFunctions << viable;
  }

  qSort( viableFunctions );

  return viableFunctions;
}

ViableFunction OverloadResolver::resolveListViable( const ParameterList& params, const QList<QPair<OverloadResolver::ParameterList, Declaration*> >& declarations, bool partial )
{
  if ( !m_context || !m_topContext )
    return ViableFunction();

  ifDebugOverloadResolution(qDebug() << "resolveListViable" << params; )
  ///Iso c++ draft 13.3.3
  m_worstConversionRank = ExactMatch;

  ///First step: Replace class-instances with operator() functions, and pure classes with their constructors
  QHash<Declaration*, OverloadResolver::ParameterList> newDeclarations;
  expandDeclarations( declarations, newDeclarations );

  ///Second step: Find best viable function
  ViableFunction bestViableFunction(m_topContext.data());

  for ( QHash<Declaration*, OverloadResolver::ParameterList>::const_iterator it = newDeclarations.constBegin(); it != newDeclarations.constEnd(); ++it )
  {
    ParameterList mergedParams = it.value();
    mergedParams.parameters += params.parameters;
    
    Declaration* decl = applyImplicitTemplateParameters( mergedParams, it.key() );
    ifDebugOverloadResolution(qDebug() << it.key()->toString() << decl; )
    if ( !decl )
      continue;
    
    ViableFunction viable( m_topContext.data(), decl, m_constness );
    viable.matchParameters( mergedParams, partial );

    ifDebugOverloadResolution(qDebug() << decl->toString() << viable.isBetter(bestViableFunction); )
    if ( viable.isBetter( bestViableFunction ) )
    {
      bestViableFunction = viable;
      m_worstConversionRank = bestViableFunction.worstConversion();
    }
  }

  return bestViableFunction;
}

Declaration* OverloadResolver::applyImplicitTemplateParameters( const ParameterList& params, Declaration* declaration ) const
{
  TemplateDeclaration* tempDecl = dynamic_cast<TemplateDeclaration*>( declaration );
  if ( !tempDecl )
    return declaration;

  KDevelop::DUContext* templateContext = tempDecl->templateParameterContext();
  if ( !templateContext )
  {
    //May be just within a template, but without own template parameters
    return declaration;
  }

  FunctionType::Ptr functionType = declaration->type<FunctionType>();
  if ( !functionType )
  {
    kDebug( 9007 ) << "Template function has no function type";
    return declaration;
  }

  const IndexedType* arguments( functionType->indexedArguments() );
  if ( static_cast<uint>(params.parameters.count()) > functionType->indexedArgumentsSize() )
    return declaration;

  //templateContext contains the template-parameters that we need to find instantiations for

  QMap<IndexedString, AbstractType::Ptr> instantiatedParameters; //Here we store the values assigned to each template-parameter

  foreach( Declaration* decl, templateContext->localDeclarations() )
  {
    CppTemplateParameterType::Ptr paramType = decl->abstractType().cast<CppTemplateParameterType>();
    if ( paramType )  //Parameters that are not of type CppTemplateParameterType are already assigned.
    {
      instantiatedParameters[decl->identifier().identifier()] = AbstractType::Ptr();
    }
  }

  if ( instantiatedParameters.isEmpty() )
    return declaration; //All parameters already have a type assigned

  for ( int a = 0; a < params.parameters.count(); a++ )
    matchParameterTypes( params.parameters[a].type, arguments[a].abstractType(), instantiatedParameters );

  bool allInstantiated = true;
  for ( QMap<IndexedString, AbstractType::Ptr>::const_iterator it = instantiatedParameters.constBegin(); it != instantiatedParameters.constEnd(); ++it )
    if ( !( *it ) )
    {
      allInstantiated = false;
      break;
    }

  if ( allInstantiated )
  {
    //We have new template-parameters at hand, we can specialize now.
    Cpp::InstantiationInformation instantiateWith( tempDecl->instantiatedWith().information() );
    instantiateWith.templateParametersList().clear();

    foreach( Declaration* decl, templateContext->localDeclarations() )
    {
      AbstractType::Ptr type;

      CppTemplateParameterType::Ptr paramType = decl->abstractType().cast<CppTemplateParameterType>();
      if ( paramType ) //Take the type we have assigned.
        type = instantiatedParameters[decl->identifier().identifier()];
      else
        type = decl->abstractType(); //Take the type that was available already earlier

      instantiateWith.addTemplateParameter( type );
    }

    return tempDecl->instantiate( instantiateWith, m_topContext.data() );
  }

  return declaration;
}

inline uint incrementIfSuccessful( uint val )
{
  if ( val )
    return 1 + val;
  return 0;
}

uint OverloadResolver::matchParameterTypes( const AbstractType::Ptr& argumentType, const AbstractType::Ptr& parameterType, QMap<IndexedString, AbstractType::Ptr>& instantiatedTypes, bool keepValue ) const
{

  if ( !argumentType && !parameterType )
    return 1;
  if ( !argumentType || !parameterType )
    return 0;

  ifDebugOverloadResolution( kDebug() << "matching" << argumentType->toString() << "to" << parameterType->toString(); )

  if ( instantiatedTypes.isEmpty() )
    return 1;

  DelayedType::Ptr delayed = parameterType.cast<DelayedType>();
  if ( delayed )
    return incrementIfSuccessful( matchParameterTypes( argumentType, delayed->identifier(), instantiatedTypes , keepValue ) );

  ///In case of references on both sides, match the target-types
  ReferenceType::Ptr argumentRef = argumentType.cast<ReferenceType>();
  ReferenceType::Ptr parameterRef = parameterType.cast<ReferenceType>();

  if ( argumentRef && parameterRef )
    return incrementIfSuccessful( matchParameterTypes( argumentRef->baseType(), parameterRef->baseType(), instantiatedTypes, keepValue ) );
  else if ( argumentRef )
    return incrementIfSuccessful( matchParameterTypes( argumentRef->baseType(), parameterType, instantiatedTypes, keepValue ) );
  else if ( parameterRef )
    return incrementIfSuccessful( matchParameterTypes( argumentType, parameterRef->baseType(), instantiatedTypes, keepValue ) );

  ///In case of pointers on both sides, match the target-types
  PointerType::Ptr argumentPointer = argumentType.cast<PointerType>();
  PointerType::Ptr parameterPointer = parameterType.cast<PointerType>();

  if ( argumentPointer && parameterPointer && (( argumentPointer->modifiers() & AbstractType::ConstModifier ) == ( parameterPointer->modifiers() & AbstractType::ConstModifier ) ) )
    return incrementIfSuccessful( matchParameterTypes( argumentPointer->baseType(), parameterPointer->baseType(), instantiatedTypes, keepValue ) );

  ///In case of arrays on both sides, match the target-types
    ArrayType::Ptr argumentArray = argumentType.cast<ArrayType>();
    ArrayType::Ptr parameterArray = parameterType.cast<ArrayType>();
    if ( argumentArray && parameterArray && (( argumentArray->modifiers() & AbstractType::ConstModifier ) == ( parameterArray->modifiers() & AbstractType::ConstModifier ) ) ) {
      return incrementIfSuccessful( matchParameterTypes( argumentArray->elementType(), parameterArray->elementType(), instantiatedTypes, keepValue ) );
    }

  if ( CppTemplateParameterType::Ptr templateParam = parameterType.cast<CppTemplateParameterType>() )
  {
    ///@todo Allow template-parameters with even more template-parameters declared
    //Directly assign argumentType to the template parameter
    Declaration* decl = templateParam->declaration( m_topContext.data() );
    if ( decl )
    {
      IndexedString id = decl->identifier().identifier();
      if ( instantiatedTypes[id] )
      {
        instantiatedTypes[id] = argumentType;
        return 1;
      }
      else if ( instantiatedTypes[id]->equals( argumentType.data() ) )
      {
        return 1;
      }
      else
      {
        //Mismatch, another type was already assigned
        return 0;
      }
    }
  }

  ///Match assigned template-parameters, for example when matching QList<int> to QList<T>, assign int to T.
  const IdentifiedType* identifiedArgument = dynamic_cast<const IdentifiedType*>( argumentType.data() );
  const IdentifiedType* identifiedParameter = dynamic_cast<const IdentifiedType*>( parameterType.data() );

  if ( identifiedArgument && identifiedParameter )
  {
    Declaration* argumentDeclaration = identifiedArgument->declaration( m_topContext.data() );
    Declaration* parameterDeclaration = identifiedParameter->declaration( m_topContext.data() );
    TemplateDeclaration* argumentTemplateDeclaration = dynamic_cast<TemplateDeclaration*>( argumentDeclaration );
    TemplateDeclaration* parameterTemplateDeclaration = dynamic_cast<TemplateDeclaration*>( parameterDeclaration );
    if ( !argumentTemplateDeclaration || !parameterTemplateDeclaration )
    {
      return (int)(argumentDeclaration == parameterDeclaration);
    }

    if ( argumentTemplateDeclaration->instantiatedFrom() == parameterTemplateDeclaration->instantiatedFrom() && argumentTemplateDeclaration->instantiatedFrom() )
    {
      InstantiationInformation argumentInstantiatedWith = argumentTemplateDeclaration->instantiatedWith().information();
      InstantiationInformation parameterInstantiatedWith = parameterTemplateDeclaration->instantiatedWith().information();

      if ( argumentInstantiatedWith.templateParametersSize() != parameterInstantiatedWith.templateParametersSize() )
        return 0;

      uint matchDepth = 1;

      for ( uint a = 0; a < argumentInstantiatedWith.templateParametersSize(); ++a )
      {
        uint localMatchDepth = matchParameterTypes( argumentInstantiatedWith.templateParameters()[a].abstractType(), parameterInstantiatedWith.templateParameters()[a].abstractType(), instantiatedTypes, keepValue );
        if ( !localMatchDepth )
          return 0;
        matchDepth += localMatchDepth;
      }

      return matchDepth;
    }
  }

  return 1;
}

QList<Declaration *> OverloadResolver::computeADLCandidates( const ParameterList& params, const QualifiedIdentifier& identifier )
{
  if(!allowADL || identifier.count() != 1 )
    return QList<Declaration *>();

  // Don't try to do ADL if there are delayed/unresolved types involved,
  // because then we cannot get a proper match as to ViableFunction anyway
  foreach( const Parameter & param, params.parameters ) {
      if( fastCast<DelayedType*>(param.type.data()) ) {
          ifDebugOverloadResolution(qDebug() << "Skipping ADL due to delayed types" << identifier << params; )
          return QList<Declaration *>();
      }
  }
  
  ADLHelper adlHelper( m_context, m_topContext );
  
  foreach( const Parameter & param, params.parameters )
    adlHelper.addArgument( param );

  QSet<QualifiedIdentifier> adlNamespaces = adlHelper.associatedNamespaces();

#ifdef DEBUG_ADL
  foreach( QualifiedIdentifier ns, adlNamespaces )
  {
    kDebug() << "  ADL found namespace: " << ns.toString();
  }
#endif

  QList<Declaration*> adlDecls;

#ifdef DEBUG_ADL
    kDebug() << "  ADL candidates for: " << identifier << params << params.parameters.size();
#endif

  foreach( QualifiedIdentifier adlFunctionName, adlNamespaces )
  {
    adlFunctionName += identifier;
    
    // By using DeclarationId, we prevent a lot of complex logic which we don't require, as we
    // already have the fully qualified scope.
    PersistentSymbolTable::FilteredDeclarationIterator decls =
        PersistentSymbolTable::self().getFilteredDeclarations(IndexedQualifiedIdentifier(adlFunctionName), m_topContext->recursiveImportIndices());
    
    for(; decls; ++decls)
    {
      Declaration* decl = decls->data();
      if(decl && decl->isFunctionDeclaration()) {
      adlDecls << decl;
#ifdef DEBUG_ADL
    kDebug() << "    ADL candidate: " << adlFunctionName << decl->toString();
#endif      
      }
    }
  }

  return adlDecls;
}

AbstractType::Ptr getContainerType( AbstractType::Ptr type, int depth, TopDUContext* topContext )
{
  for ( int a = 0; a < depth; a++ )
  {
    AbstractType::Ptr real = TypeUtils::realType( type, topContext );
    IdentifiedType* idType = dynamic_cast<IdentifiedType*>( real.data() );
    if ( !idType )
      return AbstractType::Ptr();

    Declaration* decl = idType->declaration(topContext);
    if(!decl || !decl->context())
      return AbstractType::Ptr();

    Declaration* containerDecl = decl->context()->owner();

    if ( containerDecl )
      type = containerDecl->abstractType();
    else
      return AbstractType::Ptr();
  }
  return type;
}

uint OverloadResolver::matchParameterTypes( AbstractType::Ptr argumentType, const IndexedTypeIdentifier& parameterType, QMap<IndexedString, AbstractType::Ptr>& instantiatedTypes, bool keepValue ) const
{
  ifDebugOverloadResolution( kDebug() << "1 matching" << argumentType->toString() << "to" << parameterType.toString() << parameterType.pointerDepth(); )
  if ( !argumentType )
    return 1;
  if ( instantiatedTypes.isEmpty() )
    return 1;

  QualifiedIdentifier parameterQid( parameterType.identifier().identifier() );

  if ( parameterQid.isEmpty() )
    return 1;

  {
    ReferenceType::Ptr argumentRef = argumentType.cast<ReferenceType>();

    if ( argumentRef && parameterType.isReference() )
      argumentType = argumentRef->baseType();
    else if ( parameterType.isReference() )
      return 0; //Reference on right side, but not on left
  }
  {
    PointerType::Ptr argumentPointer = argumentType.cast<PointerType>();
    int cnt = 0; ///@todo correct ordering of the pointers and their constnesses
    while ( argumentPointer && cnt < parameterType.pointerDepth() )
    {

      ++cnt;
      argumentType = argumentPointer->baseType();
      argumentPointer = argumentType.cast<PointerType>();
    }
    if ( cnt != parameterType.pointerDepth() || !argumentType )
    {
      return 0; //Do not have the needed count of pointers
    }
  }

  uint matchDepth = 1;

  if (( argumentType->modifiers() & AbstractType::ConstModifier ) && parameterType.isConstant() )
    ++matchDepth;

  for ( int a = 0; a < parameterQid.count(); ++a )
  {
    ///@todo Think about this
    AbstractType::Ptr pType = getContainerType( argumentType, parameterQid.count() - a - 1, m_topContext.data() );
    uint localDepth = matchParameterTypes( pType, parameterQid.at( a ), instantiatedTypes, keepValue );
//     if(!localDepth)
//       return 0;
    matchDepth += localDepth;
  }
  return matchDepth;
}

uint OverloadResolver::matchParameterTypes( AbstractType::Ptr argumentType, const Identifier& parameterType, QMap<IndexedString, AbstractType::Ptr>& instantiatedTypes, bool keepValue ) const
{
  ifDebugOverloadResolution( kDebug() << "2 matching" << argumentType->toString() << "to" << parameterType.toString(); )

  if ( !argumentType )
    return 1;
  if ( instantiatedTypes.isEmpty() )
    return 1;


  if ( instantiatedTypes.contains( parameterType.identifier() ) )
  {
    if ( !keepValue ) //Extract the actual type without the value
      if ( ConstantIntegralType::Ptr integral = argumentType.cast<ConstantIntegralType>() )
        argumentType = AbstractType::Ptr( new IntegralType( *integral ) );

    instantiatedTypes[parameterType.identifier()] = argumentType;
    return 1;
  }

  IdentifiedType* identifiedArgument = dynamic_cast<IdentifiedType*>( argumentType.data() );
  if ( !identifiedArgument )
    return 0;

  if ( identifiedArgument->qualifiedIdentifier().last().identifier() != parameterType.identifier() )
  {
    return 0;
  }

  TemplateDeclaration* argumentTemplateDeclaration = dynamic_cast<TemplateDeclaration*>( identifiedArgument->declaration( m_topContext.data() ) );

  uint matchDepth = 1;

  if ( argumentTemplateDeclaration && parameterType.templateIdentifiersCount() )
  {
    DUContext* argumentTemplateDeclarationContext = argumentTemplateDeclaration->templateParameterContext();

    if ( argumentTemplateDeclarationContext )
    {
      int matchLength = argumentTemplateDeclarationContext->localDeclarations().count();
      if ( parameterType.templateIdentifiersCount() < static_cast<uint>(matchLength) )
        matchLength = parameterType.templateIdentifiersCount();

      for ( int a = 0; a < matchLength; ++a )
        matchDepth += matchParameterTypes( argumentTemplateDeclarationContext->localDeclarations()[a]->abstractType(), parameterType.templateIdentifier( a ), instantiatedTypes, keepValue );
    }
    else
    {
      kDebug( 9007 ) << "Template-declaration missing template-parameter context";
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

QDebug operator<<(QDebug s, const Cpp::OverloadResolver::Parameter& param)
{
  return s << param.toString();
}

QDebug operator<<(QDebug s, const Cpp::OverloadResolver::ParameterList& params)
{
  return s << params.parameters;
}
