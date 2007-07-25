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

typedef CppDUContext<KDevelop::DUContext> StandardCppDUContext;

uint qHash( const ExpressionEvaluationResult& key ) {
  ///@todo Use the whole content for the hash
  return (uint)key.type.data();
}

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
{ ///Delete all slave-declarations
  if( m_specializedFrom ) {
    SpecializationHash::iterator it = m_specializedFrom->m_specializations.find(m_specializedWith);
    if( it != m_specializedFrom->m_specializations.end() && *it == this )
      m_specializedFrom->m_specializations.erase(it);
    
    m_specializedFrom = 0;
  }

  foreach( TemplateDeclaration* decl, m_specializations ) {
    ///Specializations should always be anonymous, so we can delete them here.
    decl->m_specializedFrom = 0;
    delete decl;
  }
}

TemplateDeclaration* TemplateDeclaration::specializedFrom() const {
  return m_specializedFrom;
}

void TemplateDeclaration::setSpecializedFrom(TemplateDeclaration* from, const QList<ExpressionEvaluationResult>& templateArguments) {
  m_specializedFrom = from;
  m_specializedWith = templateArguments;
  from->m_specializations.insert(templateArguments, this);
}

bool TemplateDeclaration::isSpecializedFrom(const TemplateDeclaration* other) const {
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

CppDUContext<KDevelop::DUContext>* specializeDeclarationContext( KDevelop::DUContext* parentContext, KDevelop::DUContext* context, const QList<ExpressionEvaluationResult>& templateArguments, Declaration* specializedDeclaration  )
{
  DUChainWriteLocker lock(DUChain::lock());
  
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
  
  if( !context ) {
    ///Note: this is possible, for example for template function-declarations(They do not have an internal context, because they have no compound statement)
    return 0;
  }

  Q_ASSERT(context->parentContext()); //Top-context is not allowed
  StandardCppDUContext* contextCopy = new StandardCppDUContext(context->textRangePtr(), parentContext, true); //We do not need to care about TopDUContext here, because a top-context can not be specialized
  contextCopy->setRangeOwning(KDevelop::DocumentRangeObject::DontOwn); //The range belongs to the original context, so flag it not to be owned by the context
  contextCopy->setType(context->type());
  contextCopy->setLocalScopeIdentifier(context->localScopeIdentifier());

  kDebug() << "Created context " << contextCopy << " as specialization of context " << context << endl;
  
  if( specializedDeclaration )
    specializedDeclaration->setInternalContext(contextCopy);

  contextCopy->setSpecializedFrom(context);

  ///Now the created context is already partially functional and can be used for searching(not the specialized template-params though)
  
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

  lock.unlock();
  
  foreach( const KDevelop::DUContext* importedContext,  context->importedParentContexts() )
  {///@todo remove the const_cast's
      ///For functions, the Template-context is one level deeper(it is imported by the function-context) so also copy the function-context
    if( importedContext->type() == KDevelop::DUContext::Template || importedContext->type() == KDevelop::DUContext::Function )
    {
      DUContext* ctx = specializeDeclarationContext( parentContext, const_cast<KDevelop::DUContext*>(importedContext), templateArguments, 0);
      lock.lock();
      contextCopy->addImportedParentContext( ctx );
      lock.unlock();
    }
    else
    {
      ///@todo resolve template-dependent base-classes(They can not be found here, because their type is DelayedType and those have no context)
      //Import all other imported contexts
      lock.lock();
      contextCopy->addImportedParentContext(const_cast<KDevelop::DUContext*>(importedContext));
      lock.unlock();
    }
  }

  return contextCopy;
}

///@todo register specialized declarations, re-use them, find best matching specialization, delete them when this class is deleted
Declaration* TemplateDeclaration::specialize( const QList<ExpressionEvaluationResult>& templateArguments ) {

  SpecializationHash::const_iterator it = m_specializations.find(templateArguments);

  if( it != m_specializations.end() )
    return dynamic_cast<Declaration*>(*it);
  
  Declaration* decl = dynamic_cast<Declaration*>(this);
  Q_ASSERT(decl);
  Declaration* clone = decl->clone();
  Q_ASSERT(clone);


  if( decl->abstractType() ) {
    ///Also specialize the type(This usually just means that the declaration is changed, but in case of a function the arguments may be changed)
    AbstractType::Ptr typeCopy( resolveDelayedTypes( decl->abstractType(), decl->internalContext() ? decl->internalContext() : decl->context() ) );
    DUChainWriteLocker lock(DUChain::lock());

    if( typeCopy == decl->abstractType() ) //It must be a copy, so we can change the declaration
      typeCopy = decl->abstractType()->clone();
    
    IdentifiedType* idType = dynamic_cast<IdentifiedType*>(typeCopy.data());
    if( idType )
      idType->setDeclaration(clone);
    
    clone->setAbstractType(typeCopy);
  }

  TemplateDeclaration* cloneTemplateDecl = dynamic_cast<TemplateDeclaration*>(clone);
  Q_ASSERT(cloneTemplateDecl);
  
  ///Now eventually create the virtual contexts
  specializeDeclarationContext( decl->context(), decl->internalContext(), templateArguments, clone );
  
  cloneTemplateDecl->setSpecializedFrom(this, templateArguments);
  
  return clone;
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

/*    AbstractType::Ptr temp( resolver.exchange(type.data()) );
    if( temp ) //The whole type was a delayed type
      return temp;
*/
    DUChainWriteLocker lock(DUChain::lock());

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

