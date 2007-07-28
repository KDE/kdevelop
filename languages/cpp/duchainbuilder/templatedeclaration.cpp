/* This file is part of KDevelop
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


#include "templatedeclaration.h"
#include "templateparameterdeclaration.h"
#include "cppducontext.h"

using namespace KDevelop;
using namespace Cpp;

QMutex TemplateDeclaration::specializationsMutex;

typedef CppDUContext<KDevelop::DUContext> StandardCppDUContext;

uint qHash( const ExpressionEvaluationResult& key ) {
  ///@todo Use the whole content for the hash
  return (uint)key.type.data();
}



///Finds out whether any DelayedType's are involved in a given type(It searches return-values, argument-types, base-classes, etc.)
struct DelayedTypeSearcher : public KDevelop::SimpleTypeVisitor {
  bool found;

  DelayedTypeSearcher() : found(false) {
  }

  virtual bool visit ( const AbstractType* type ) {
    if( found ) return false;
    if( dynamic_cast<const DelayedType*>(type) )
      found = true;
    return !found;
  }

  virtual bool visit (const StructureType *) {
    ///We do not want to visit member-types, so return false here
    return false;
  }
};

/**
 * Returns whether any count of reference/pointer-types are followed by a delayed type
 * */
const DelayedType* containsDelayedType(const AbstractType* type)
{
  const PointerType* pType = dynamic_cast<const PointerType*>(type);
  const ReferenceType* rType = dynamic_cast<const ReferenceType*>(type);
  const DelayedType* delayedType = dynamic_cast<const DelayedType*>(type);
  if( pType )
    return containsDelayedType(pType->baseType().data());
  if( rType )
    return containsDelayedType(rType->baseType().data());
  return delayedType;
}

///Replaces any DelayedType's in interesting positions with their resolved versions, if they can be resolved.
struct DelayedTypeResolver : public KDevelop::TypeExchanger {
  const KDevelop::DUContext* searchContext;

  DelayedTypeResolver(const KDevelop::DUContext* _searchContext) : searchContext(_searchContext) {
  }

  virtual AbstractType* exchange( const AbstractType* type )
  {
    const DelayedType* delayedType = dynamic_cast<const DelayedType*>(type);

    if( delayedType ) {
      QList<Declaration*> decls = searchContext->findDeclarations(delayedType->qualifiedIdentifier(), KTextEditor::Cursor::invalid(), AbstractType::Ptr(), KDevelop::DUContext::NoUndefinedTemplateParams );
      if( !decls.isEmpty() ) {
        return decls.front()->abstractType().data();
      }
    }else{
      if( containsDelayedType(type) )
      {
        //Copy the type to keep the correct reference/pointer structure
        AbstractType* typeCopy = type->clone();
        PointerType* pType = dynamic_cast<PointerType*>(typeCopy);
        ReferenceType* rType = dynamic_cast<ReferenceType*>(typeCopy);
        if( pType ) //Replace the base
          pType->exchangeTypes(this);
        if( rType ) //Replace the base
          rType->exchangeTypes(this);
        
        return typeCopy;
      }
      
    }
    
    return const_cast<AbstractType*>(type); ///@todo remove const_cast;
  }

  virtual bool exchangeMembers() const {
    return false;
  }
};

bool operator==( const ExpressionEvaluationResult& left, const ExpressionEvaluationResult& right ) {
 return left.type == right.type; ///@todo use the other parts
}

uint qHash( const QList<ExpressionEvaluationResult>& key ) {
  uint ret = 0;
  foreach( const ExpressionEvaluationResult& expr, key )
    ret = qHash(expr) + ret * 7;
  return ret;
}

namespace Cpp {
// bool operator==( const QList<ExpressionEvaluationResult>& left, const QList<ExpressionEvaluationResult>& right ) {
//   return true;
// }

TemplateDeclaration::TemplateDeclaration(const TemplateDeclaration& /*rhs*/) : m_parameterContext(0), m_specializedFrom(0) {
}

TemplateDeclaration::TemplateDeclaration() : m_parameterContext(0), m_specializedFrom(0) {
}

TemplateDeclaration::~TemplateDeclaration()
{ 
  
  SpecializationHash specializations;
  {
    QMutexLocker l(&specializationsMutex);
    specializations = m_specializations;

    ///Unregister at the declaration this one is specialized from
    if( m_specializedFrom ) {
      SpecializationHash::iterator it = m_specializedFrom->m_specializations.find(m_specializedWith);
      if( it != m_specializedFrom->m_specializations.end() && *it == this )
        m_specializedFrom->m_specializations.erase(it);
      
      m_specializedFrom = 0;
    }
  }

  ///Delete all slave-declarations
  foreach( TemplateDeclaration* decl, specializations ) {
    ///Specializations should always be anonymous, so we can delete them here.
    decl->m_specializedFrom = 0;
    delete decl;
  }
}

TemplateDeclaration* TemplateDeclaration::specializedFrom() const {
  return m_specializedFrom;
}

void TemplateDeclaration::setSpecializedFrom(TemplateDeclaration* from, const QList<ExpressionEvaluationResult>& templateArguments) {
  QMutexLocker l(&specializationsMutex);
  if( m_specializedFrom )
    m_specializedFrom->m_specializations.remove(m_specializedWith);
  m_specializedFrom = from;
  m_specializedWith = templateArguments;
  from->m_specializations.insert(templateArguments, this);
}

bool TemplateDeclaration::isSpecializedFrom(const TemplateDeclaration* other) const {
    QMutexLocker l(&specializationsMutex);
  
    SpecializationHash::const_iterator it = other->m_specializations.find(m_specializedWith);
    if( it != other->m_specializations.end() && (*it) == this )
      return true;
    else
      return false;
}


void TemplateDeclaration::setTemplateParameterContext(KDevelop::DUContext* context) {
  m_parameterContext = context;
}

KDevelop::DUContext* TemplateDeclaration::templateParameterContext() const {
  return const_cast<KDevelop::DUContext*>(m_parameterContext.data()); ///@todo make data() const; return non-const pointer in duchain-pointer
}

bool isTemplateDeclaration(const KDevelop::Declaration* decl) {
  return (bool)dynamic_cast<const TemplateDeclaration*>(decl);
}

///@todo prevent endless recursion when resolving base-classes!(Parent is not yet in du-chain, so a base-class that references it will cause endless recursion)
CppDUContext<KDevelop::DUContext>* specializeDeclarationContext( KDevelop::DUContext* parentContext, KDevelop::DUContext* context, const QList<ExpressionEvaluationResult>& templateArguments, Declaration* specializedDeclaration, Declaration* specializedFrom )
{
  if( specializedDeclaration ) {
    ///Move the specialized declaration anonymously into the parent-context
    specializedDeclaration->setContext(parentContext, true);

    if( !templateArguments.isEmpty() ) {
      ///Change the identifier to reflect the set template-arguments @todo use default-parameters
      KDevelop::Identifier id = specializedDeclaration->identifier();
      foreach(Cpp::ExpressionEvaluationResult expr, templateArguments) {
        KDevelop::IdentifiedType* idType = dynamic_cast<KDevelop::IdentifiedType*>(expr.type.data());
        
        if( idType )
          id.appendTemplateIdentifier(idType->identifier());
        ///@todo reenable once expressionparser und duchainbuilder are merged
/*        else
          id.appendTemplateIdentifier(QualifiedIdentifier(expr.toString()));*/
      }
      
      specializedDeclaration->setIdentifier(id);
    }
  }

  StandardCppDUContext* contextCopy = 0;
  
  if( context ) {
    ///Specialize involved contexts
    Q_ASSERT(context->parentContext()); //Top-context is not allowed
    contextCopy = new StandardCppDUContext(context->textRangePtr(), parentContext, true); //We do not need to care about TopDUContext here, because a top-context can not be specialized
    contextCopy->setRangeOwning(KDevelop::DocumentRangeObject::DontOwn); //The range belongs to the original context, so flag it not to be owned by the context
    contextCopy->setType(context->type());
    contextCopy->setLocalScopeIdentifier(context->localScopeIdentifier());

    kDebug() << "Created context " << contextCopy << " as specialization of context " << context << endl;
    
    if( specializedDeclaration )
      specializedDeclaration->setInternalContext(contextCopy);

    
    ///Now the created context is already partially functional and can be used for searching(not the specialized template-params yet though)
    
    if(context->type() == KDevelop::DUContext::Template && !templateArguments.isEmpty()) { //templateArguments may be empty, that means that only copying should happen.
      ///Specialize the local template-declarations

      QList<ExpressionEvaluationResult>::const_iterator currentArgument = templateArguments.begin();
      
      foreach(Declaration* decl, context->localDeclarations())
      {
        TemplateParameterDeclaration* templateDecl = dynamic_cast<TemplateParameterDeclaration*>(decl);
        Q_ASSERT(templateDecl); //Only template-parameter declarations are allowed in template-contexts
        TemplateParameterDeclaration* declCopy = dynamic_cast<TemplateParameterDeclaration*>(decl->clone());
        Q_ASSERT(declCopy);

        if( currentArgument != templateArguments.end() )
        {
          declCopy->setAbstractType( currentArgument->type );
          
          ++currentArgument;
        } else {
          //templateDecl->defaultParameter()
          ///@todo Use default-parameters! Use the expression-parser here to resolve the default-parameters(If the default-parameter is not a valid qualified identifier)
          kDebug() << "missing template-argument" << endl;
        }
        ///This inserts the copied declaration into the copied context
        declCopy->setContext(contextCopy);
      }
    }
    
    foreach( KDevelop::DUContextPointer importedContext,  context->importedParentContexts() )
    {
      if( !importedContext)
        continue;
        ///For functions, the Template-context is one level deeper(it is imported by the function-context) so also copy the function-context
      if( importedContext->type() == KDevelop::DUContext::Template || importedContext->type() == KDevelop::DUContext::Function )
      {
        DUContext* ctx = specializeDeclarationContext( parentContext, importedContext.data(), templateArguments, 0, 0);
        contextCopy->addImportedParentContext( ctx, true );
      }
      else
      {
        //Import all other imported contexts
        contextCopy->addImportedParentContext( importedContext.data(), true );
      }
    }

    if( specializedDeclaration ) {
      const CppClassType* klass = dynamic_cast<const CppClassType*>( specializedDeclaration->abstractType().data() );
      if( klass ) { //It could also be a function
        ///Resolve template-dependent base-classes(They can not be found in the imports-list, because their type is DelayedType and those have no context)

        foreach( const CppClassType::BaseClassInstance& base, klass->baseClasses() ) {
          const DelayedType* delayed = dynamic_cast<const DelayedType*>(base.baseClass.data());
          if( delayed ) {
            ///Resolve the delayed type, and import the context
            DelayedTypeResolver res(contextCopy);
            AbstractType::Ptr newType( res.exchange(delayed) );

            if( CppClassType* baseClass = dynamic_cast<CppClassType*>(newType.data()) )
            {
              if( baseClass->declaration() && baseClass->declaration()->internalContext() )
              {
                contextCopy->addImportedParentContext( baseClass->declaration()->internalContext(), true );
              }
            } else {
              kDebug() << "Resolved bad base-class" << endl;
            }
          }
        }
      }
    }

  } else {
    ///Note: this is possible, for example for template function-declarations(They do not have an internal context, because they have no compound statement), for variables, etc.
  }

  if( contextCopy )
    contextCopy->setSpecializedFrom(dynamic_cast<CppDUContext<DUContext>*>(context));
  ///Since now the context is accessible through the du-chain, so it must not be changed any more.
    
  if( specializedDeclaration && specializedDeclaration->abstractType() ) {
    IdentifiedType* idType = dynamic_cast<IdentifiedType*>(specializedDeclaration->abstractType().data());

    ///Use the internal context if it exists, so undefined template-arguments can be found and the DelayedType can be further delayed then.
    AbstractType::Ptr changedType = resolveDelayedTypes( specializedDeclaration->abstractType(), specializedDeclaration->internalContext() ? specializedDeclaration->internalContext() : parentContext );

    if( changedType == specializedDeclaration->abstractType() )
      if( idType && idType->declaration() == specializedFrom ) { //We must clone it, so we can change IdentifiedType::declaration
        changedType = specializedDeclaration->abstractType()->clone();

        IdentifiedType* changedIdType = dynamic_cast<IdentifiedType*>(changedType.data());
        Q_ASSERT(changedIdType);
        changedIdType->setDeclaration(specializedDeclaration);
      }
    
    specializedDeclaration->setAbstractType( changedType );
  }
  
  return contextCopy;
}

///@todo Use explicitly declared specializations
Declaration* TemplateDeclaration::specialize( const QList<ExpressionEvaluationResult>& templateArguments )
{
  {
    QMutexLocker l(&specializationsMutex);
    SpecializationHash::const_iterator it;
    it = m_specializations.find(templateArguments);
    if( it != m_specializations.end() )
      return dynamic_cast<Declaration*>(*it);
  }

  
  Declaration* decl = dynamic_cast<Declaration*>(this);
  Q_ASSERT(decl);
  Declaration* clone = decl->clone();
  Q_ASSERT(clone);
  TemplateDeclaration* cloneTemplateDecl = dynamic_cast<TemplateDeclaration*>(clone);
  Q_ASSERT(cloneTemplateDecl);
  
  ///Now eventually create the virtual contexts
  specializeDeclarationContext( decl->context(), decl->internalContext(), templateArguments, clone, decl );

  cloneTemplateDecl->setSpecializedFrom(this, templateArguments);
  
  return clone;
}

AbstractType::Ptr resolveDelayedTypes( AbstractType::Ptr type, const KDevelop::DUContext* context ) {
  if( !type )
    return type;

  ///First, find out if delayed types are involved(Function return-type, function-arguments, reference/pointer target, base-class)
  DelayedTypeSearcher search;
  
  type->accept(&search);

  DelayedType* delayedType = dynamic_cast<DelayedType*>(type.data());
  
  if( search.found || delayedType ) {
    ///Delayed types were found. We must copy the whole type, and replace the delayed types.

    DelayedTypeResolver resolver(context);

    AbstractType::Ptr typeCopy;
    if( delayedType )
      ///The type itself is a delayed type, resolve it
      typeCopy = resolver.exchange( type.data() );
    else {
      ///Resolve involved delayed types, now hopefully we know the template-parameters
      typeCopy = AbstractType::Ptr( type->clone() );
      DelayedTypeSearcher testSearch;
      typeCopy->accept(&testSearch);
      Q_ASSERT(testSearch.found);
      typeCopy->exchangeTypes( &resolver );
    }

    return typeCopy;
  } else {
    return type;
  }
}

}

