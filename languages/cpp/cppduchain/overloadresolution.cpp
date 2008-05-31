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
#include "duchain/ducontext.h"
#include "duchain/declaration.h"
#include "duchain/classfunctiondeclaration.h"
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
    QList<Declaration*> declarations = m_context->findLocalDeclarations(m_context->localScopeIdentifier(), KDevelop::SimpleCursor(), m_topContext.data(), AbstractType::Ptr(), DUContext::OnlyFunctions);
    
    for( QList<Declaration*>::iterator it = declarations.begin(); it != declarations.end(); ++it ) {
      if( (*it)->abstractType() )
      {
        CppFunctionType* function = dynamic_cast<CppFunctionType*>( (*it)->abstractType().data() );
        ClassFunctionDeclaration* functionDeclaration = dynamic_cast<ClassFunctionDeclaration*>(*it);
        //Q_ASSERT();
        
        if( functionDeclaration && functionDeclaration->isConstructor() )
        {
          if( function->arguments().size() >= params.parameters.size() )
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

    if( CppClassType* klass = dynamic_cast<CppClassType*>( TypeUtils::realType(decl->abstractType(), m_topContext.data(), &isConstant) ) )
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

    if( CppClassType* klass = dynamic_cast<CppClassType*>( TypeUtils::realType(decl.second->abstractType(), m_topContext.data(), &isConstant) ) )
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
    return dynamic_cast<Declaration*>(bestViableFunction.declaration().data());
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
    kDebug(9007) << "Template function missing template context";
    return declaration;
  }

  CppFunctionType::Ptr functionType = declaration->type<CppFunctionType>();
  if(!functionType) {
    kDebug(9007) << "Template function has no function type";
    return declaration;
  }

  const QList<AbstractType::Ptr>& arguments( functionType->arguments() );
  if(params.parameters.count() > arguments.count())
    return declaration;
  
  //templateContext contains the template-parameters that we need to find instantiations for

  QMap<QString, AbstractType::Ptr> instantiatedParameters; //Here we store the values assigned to each template-parameter

  foreach( Declaration* decl, templateContext->localDeclarations() ) {
    CppTemplateParameterType* paramType = dynamic_cast<CppTemplateParameterType*>(decl->abstractType().data());
    if( paramType ) //Parameters that are not of type CppTemplateParameterType are already assigned.
      instantiatedParameters[paramType->identifier().last().identifier()] = AbstractType::Ptr();
  }

  if( instantiatedParameters.isEmpty() )
    return declaration; //All parameters already have a type assigned
  
  for( int a = 0; a < params.parameters.count(); a++ )
    matchParameterTypes(const_cast<AbstractType*>(params.parameters[a].type.data()), const_cast<AbstractType*>(arguments[a].data()), instantiatedParameters);

  bool allInstantiated = true;
  for( QMap<QString, AbstractType::Ptr>::const_iterator it = instantiatedParameters.begin(); it != instantiatedParameters.end(); ++it )
    if( !(*it) )
      allInstantiated = false;

  if( allInstantiated ) {
    //We have new template-parameters at hand, we can specialize now.
    QList<ExpressionEvaluationResult> instantiateWith;
    foreach( Declaration* decl, templateContext->localDeclarations() ) {
      ExpressionEvaluationResult res;
      
      CppTemplateParameterType* paramType = dynamic_cast<CppTemplateParameterType*>(decl->abstractType().data());
      if( paramType ) //Take the type we have assigned.
        res.type = instantiatedParameters[paramType->identifier().last().identifier()];
      else
        res.type = decl->abstractType(); //Take the type that was available already earlier

      instantiateWith << res;
    }
    return tempDecl->instantiate( instantiateWith, m_topContext->importTrace(declaration->topContext()) );
  }
  
  return declaration;
}

bool OverloadResolver::matchParameterTypes(AbstractType* _argumentType, AbstractType* _parameterType, QMap<QString, AbstractType::Ptr>& instantiatedTypes) const
{
  //kDebug() << "matching" << _argumentType->toString() << "to" << _parameterType->toString();
  
  if(!_argumentType || !_parameterType)
    return true;
  if(instantiatedTypes.isEmpty())
    return true;
  AbstractType::Ptr argumentType = TypeUtils::resolvedType(_argumentType, m_topContext.data());
  AbstractType::Ptr parameterType = TypeUtils::resolvedType(_parameterType, m_topContext.data());
  
  DelayedType* delayed = dynamic_cast<DelayedType*>(parameterType.data());
  if(delayed)
    return matchParameterTypes( argumentType.data(), delayed->identifier(), instantiatedTypes );

  ///In case of references on both sides, match the target-types
  CppReferenceType* argumentRef = dynamic_cast<CppReferenceType*>(argumentType.data());
  CppReferenceType* parameterRef = dynamic_cast<CppReferenceType*>(parameterType.data());

  if( argumentRef && parameterRef )
    return matchParameterTypes( argumentRef->baseType().data(), parameterRef->baseType().data(), instantiatedTypes );
  else if(argumentRef)
    return matchParameterTypes( argumentRef->baseType().data(), parameterType.data(), instantiatedTypes );
  else if(parameterRef)
    return matchParameterTypes( argumentType.data(), parameterRef->baseType().data(), instantiatedTypes );

  ///In case of pointers on both sides, match the target-types
  CppPointerType* argumentPointer = dynamic_cast<CppPointerType*>(argumentType.data());
  CppPointerType* parameterPointer = dynamic_cast<CppPointerType*>(parameterType.data());

  if( argumentPointer && parameterPointer )
    return matchParameterTypes( argumentPointer->baseType().data(), parameterPointer->baseType().data(), instantiatedTypes );

  ///Match assigned template-parameters, for example when matching QList<int> to QList<T>, assign int to T.
  IdentifiedType* identifiedArgument = dynamic_cast<IdentifiedType*>(argumentType.data());
  IdentifiedType* identifiedParameter = dynamic_cast<IdentifiedType*>(parameterType.data());

  if( identifiedArgument && identifiedParameter )
  {
    if( identifiedArgument->identifier() == identifiedParameter->identifier() )
    {
      TemplateDeclaration* argumentTemplateDeclaration = dynamic_cast<TemplateDeclaration*>(identifiedArgument->declaration());
      TemplateDeclaration* parameterTemplateDeclaration = dynamic_cast<TemplateDeclaration*>(identifiedParameter->declaration());

      if( argumentTemplateDeclaration && parameterTemplateDeclaration )
      {
        DUContext* argumentTemplateDeclarationContext = argumentTemplateDeclaration->templateParameterContext();
        DUContext* parameterTemplateDeclarationContext = parameterTemplateDeclaration->templateParameterContext();

        if( argumentTemplateDeclarationContext->localDeclarations().count() == parameterTemplateDeclarationContext->localDeclarations().count() ) {
          for( int a = 0; a < argumentTemplateDeclarationContext->localDeclarations().count(); ++a )
            matchParameterTypes( argumentTemplateDeclarationContext->localDeclarations()[a]->abstractType().data(), parameterTemplateDeclarationContext->localDeclarations()[a]->abstractType().data(), instantiatedTypes );
        }
      }
    }
  }
  
  return true;
}

AbstractType* getContainerType(AbstractType* type, int depth, TopDUContext* topContext) {
  for(int a = 0; a < depth; a++) {
    IdentifiedType* idType = dynamic_cast<IdentifiedType*>(TypeUtils::realType(type, topContext));
    if(!idType)
      return 0;
    
    Declaration* decl = idType->declaration();
    Declaration* containerDecl = decl->context()->owner();
    
    if(containerDecl)
      type = containerDecl->abstractType().data();
    else
      return 0;
  }
  return type;
}

bool OverloadResolver::matchParameterTypes(AbstractType* _argumentType, const TypeIdentifier& parameterType, QMap<QString, AbstractType::Ptr>& instantiatedTypes) const
{
  //kDebug() << "matching" << _argumentType->toString() << "to" << parameterType.toString();
  if(!_argumentType)
    return true;
  if(instantiatedTypes.isEmpty())
    return true;
  if(parameterType.isEmpty())
    return true;

  {
    CppReferenceType* argumentRef = dynamic_cast<CppReferenceType*>(_argumentType);

    if( argumentRef && parameterType.isReference() )
      _argumentType = argumentRef->baseType().data();
    else if( parameterType.isReference() )
      return false; //Reference on right side, but not on left
  }
  {
    CppPointerType* argumentPointer = dynamic_cast<CppPointerType*>(_argumentType);
    int cnt = 0; ///@todo correct ordering of the pointers and their constnesses
    while( argumentPointer && cnt < parameterType.pointerDepth() ) {
      ++cnt;
      _argumentType = argumentPointer->baseType().data();
      argumentPointer = dynamic_cast<CppPointerType*>(_argumentType);
    }
    if( cnt != parameterType.pointerDepth() || !_argumentType )
      return false; //Do not have the needed count of pointers
  }
  
  for( int a = 0; a < parameterType.count(); ++a ) {
    AbstractType* pType = getContainerType(_argumentType, parameterType.count() - a - 1, m_topContext.data());
    if(!matchParameterTypes(pType, parameterType.at(a), instantiatedTypes))
      return false;
  }
  return true;
}

bool OverloadResolver::matchParameterTypes(AbstractType* _argumentType, const Identifier& parameterType, QMap<QString, AbstractType::Ptr>& instantiatedTypes) const
{
  //kDebug() << "matching" << _argumentType->toString() << "to" << parameterType.toString();
  
  if(!_argumentType)
    return true;
  if(instantiatedTypes.isEmpty())
    return true;

  AbstractType::Ptr argumentType = TypeUtils::resolvedType(_argumentType, m_topContext.data());
  
  IdentifiedType* identifiedArgument = dynamic_cast<IdentifiedType*>(argumentType.data());

  if( !identifiedArgument )
    return true;

  KDevelop::Identifier argumentId = identifiedArgument->identifier().last();

  if( instantiatedTypes.contains(parameterType.identifier()) ) {
    instantiatedTypes[parameterType.identifier()] = AbstractType::Ptr(_argumentType);
    return true;
  } else if( argumentId.identifier() != parameterType.identifier() )
    return false;
  
  TemplateDeclaration* argumentTemplateDeclaration = dynamic_cast<TemplateDeclaration*>(identifiedArgument->declaration());

  if( argumentTemplateDeclaration && !parameterType.templateIdentifiers().isEmpty() )
  {
    DUContext* argumentTemplateDeclarationContext = argumentTemplateDeclaration->templateParameterContext();

    if( argumentTemplateDeclarationContext )
    {
      int matchLength = argumentTemplateDeclarationContext->localDeclarations().count();
      if( parameterType.templateIdentifiers().count() < matchLength )
        matchLength = parameterType.templateIdentifiers().count();
    
      for( int a = 0; a < matchLength; ++a )
        matchParameterTypes( argumentTemplateDeclarationContext->localDeclarations()[a]->abstractType().data(), parameterType.templateIdentifiers()[a], instantiatedTypes );
    }else{
      kDebug(9007) << "Template-declaration missing template-parameter context";
    }
  }

  return true;
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
//   CppReferenceType* argumentRef = dynamic_cast<CppReferenceType*>(argumentType.data());
//   CppReferenceType* parameterRef = dynamic_cast<CppReferenceType*>(parameterType.data());
// 
//   if( argumentRef && parameterRef )
//     return matchParameterTypes( argumentRef->baseType().data(), parameterRef->baseType().data(), instantiatedTypes );
// 
//   ///In case of pointers on both sides, match the target-types
//   CppPointerType* argumentPointer = dynamic_cast<CppPointerType*>(argumentType.data());
//   CppPointerType* parameterPointer = dynamic_cast<CppPointerType*>(parameterType.data());
// 
//   if( argumentPointer && parameterPointer )
//     return matchParameterTypes( argumentPointer->baseType().data(), parameterPointer->baseType().data(), instantiatedTypes );
//   
//   return true;
// }
