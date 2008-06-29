
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

#include <qatomic.h>
#include <kglobal.h>

#include <duchain/declaration.h>
#include <duchain/forwarddeclaration.h>
#include <duchain/repositories/itemrepository.h>
#include <appendedlist.h>

#include "templateparameterdeclaration.h"
#include "cppducontext.h"
#include "expressionparser.h"

using namespace KDevelop;
using namespace Cpp;

QMutex TemplateDeclaration::instantiationsMutex(QMutex::Recursive);

typedef CppDUContext<KDevelop::DUContext> StandardCppDUContext;

namespace Cpp {
  DEFINE_LIST_MEMBER_HASH(InstantiationInformation, templateParameters, IndexedExpressionEvaluationResult)
  
  uint qHash(const Cpp::IndexedInstantiationInformation& info) {
    return info.hash();
  }
}

struct AtomicIncrementer {
#if QT_VERSION < 0x040400
  typedef QBasicAtomic Int;
#else
  typedef QBasicAtomicInt Int;
#endif
  AtomicIncrementer( Int *cnt ) : c(cnt) {
    c->ref();
  }

  ~AtomicIncrementer() {
    c->deref();
  }

  Int* c;
};

QString InstantiationInformation::toString() const {
    QString ret;
    if(previousInstantiationInformation)
        ret = IndexedInstantiationInformation(previousInstantiationInformation).information().toString() + "::";
    ret += "<";
    for(int a = 0; a < templateParametersSize(); ++a) {
        if(a)
            ret += ", ";
        ret += templateParameters()[a].result().toString();
    }
    ret += ">";
    return ret;
}

InstantiationInformation::InstantiationInformation() : previousInstantiationInformation(0) {
  initializeAppendedLists();
}

InstantiationInformation::InstantiationInformation(const InstantiationInformation& rhs) : previousInstantiationInformation(rhs.previousInstantiationInformation) {
  initializeAppendedLists();
  copyListsFrom(rhs);
}

InstantiationInformation::~InstantiationInformation() {
  freeAppendedLists();
}

InstantiationInformation& InstantiationInformation::operator=(const InstantiationInformation& rhs) {
  previousInstantiationInformation = rhs.previousInstantiationInformation;
  copyListsFrom(rhs);
  return *this;
}

bool InstantiationInformation::operator==(const InstantiationInformation& rhs) const {
  if(previousInstantiationInformation != rhs.previousInstantiationInformation)
    return false;
  return listsEqual(rhs);
}

uint InstantiationInformation::hash() const {
  uint ret = 0;
  FOREACH_FUNCTION(const IndexedExpressionEvaluationResult& param, templateParameters) {
    ret = (ret + param.hash()) * 117;
  }
  
  return (ret + previousInstantiationInformation) * 31;
}

KDevelop::ItemRepository<InstantiationInformation, AppendedListItemRequest<InstantiationInformation> > instantiationInformationRepository("C++ Instantiation Information Repository");

const uint standardInstantiationInformationIndex = instantiationInformationRepository.index( InstantiationInformation() );

IndexedInstantiationInformation::IndexedInstantiationInformation() : m_index(standardInstantiationInformationIndex) {
}

IndexedInstantiationInformation::IndexedInstantiationInformation(uint index) : m_index(index) {
}

bool IndexedInstantiationInformation::isValid() const {
  return m_index != standardInstantiationInformationIndex;
}


const InstantiationInformation& IndexedInstantiationInformation::information() const {
  return *instantiationInformationRepository.itemFromIndex(m_index);
}

IndexedInstantiationInformation InstantiationInformation::indexed() const {
  return IndexedInstantiationInformation(instantiationInformationRepository.index(*this));
}

AbstractType* applyPointerReference( AbstractType* ptr, const KDevelop::TypeIdentifier& id ) {
  AbstractType* ret = ptr;
  for( int a = 0; a < id.pointerDepth(); ++a ) {
    Declaration::CVSpec spec = Declaration::CVNone;
    if( id.isConstPointer( a ) )
      spec = Declaration::Const;
    
    CppPointerType* newRet = new CppPointerType( spec );
    newRet->setBaseType( AbstractType::Ptr(ret) );
    ret = newRet;
  }

  if(id.isReference() ) {
    Declaration::CVSpec spec = Declaration::CVNone;
    if( id.isConstant() )
      spec = Declaration::Const;
    
    CppReferenceType* newRet = new CppReferenceType( spec );
    newRet->setBaseType( AbstractType::Ptr(ret) );
    ret = newRet;
  }

  return ret;
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
    if( const CppClassType* classType = dynamic_cast<const CppClassType*>(type) ) {
        foreach( const CppClassType::BaseClassInstance& base, classType->baseClasses() )
          visit( base.baseClass.data() );
    }
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
  const CppClassType* classType = dynamic_cast<const CppClassType*>(type);
  if( pType )
    return containsDelayedType(pType->baseType().data());
  if( rType )
    return containsDelayedType(rType->baseType().data());
  if( classType ) {
    foreach( const CppClassType::BaseClassInstance& base, classType->baseClasses() ) {
      const DelayedType* delayed = dynamic_cast<const DelayedType*>(base.baseClass.data());
      if( delayed )
        return delayed;
      else if( (delayed = containsDelayedType( base.baseClass.data() )) )
        return delayed;
    }
  }
  return delayedType;
}

///Replaces any DelayedType's in interesting positions with their resolved versions, if they can be resolved.
struct DelayedTypeResolver : public KDevelop::TypeExchanger {
  const KDevelop::DUContext* searchContext;
  
  const KDevelop::TopDUContext* source;
  static AtomicIncrementer::Int depth_counter;
  KDevelop::DUContext::SearchFlags searchFlags;

  DelayedTypeResolver(const KDevelop::DUContext* _searchContext, const KDevelop::TopDUContext* _source, KDevelop::DUContext::SearchFlags _searchFlags = KDevelop::DUContext::NoUndefinedTemplateParams) : searchContext(_searchContext), source(_source), searchFlags(_searchFlags) {
  }

  virtual AbstractType* exchange( const AbstractType* type )
  {
    AtomicIncrementer inc(&depth_counter);
    if( depth_counter > 20 ) {
      kDebug(9007) << "Too much depth in DelayedTypeResolver::exchange, while exchanging" << (type ? type->toString() : QString("(null)"));
      return const_cast<AbstractType*>(type); ///@todo remove const_cast;
    }
    const DelayedType* delayedType = dynamic_cast<const DelayedType*>(type);

    if( delayedType && delayedType->kind() == DelayedType::Delayed ) {
      if( !delayedType->identifier().isExpression() ) {
        DUContext::SearchItem::PtrList identifiers;
        identifiers << DUContext::SearchItem::Ptr( new DUContext::SearchItem(delayedType->identifier()) );
        DUContext::DeclarationList decls;
        
        if( !searchContext->findDeclarationsInternal( identifiers, searchContext->range().end, AbstractType::Ptr(), decls, source, searchFlags ) )
          return const_cast<AbstractType*>(type);
        
        if( !decls.isEmpty() )
          return applyPointerReference(decls[0]->abstractType().data(), delayedType->identifier());
      }
      ///Resolution as type has failed, or is not appropriate.
      ///Resolve delayed expression, for example static numeric expressions
      ExpressionParser p;
      ExpressionEvaluationResult res;
      if( delayedType->identifier().isExpression() )
        res = p.evaluateExpression( delayedType->identifier().toString().toUtf8(), DUContextPointer(const_cast<DUContext*>(searchContext)), source );
      else
        res = p.evaluateType( delayedType->identifier().toString().toUtf8(), DUContextPointer(const_cast<DUContext*>(searchContext)), source );

      ///@todo make this nicer
      keepAlive = res.type.type(); //Since the API uses AbstractType*, use keepAlive to make sure the type cannot be deleted
      return keepAlive.data();
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
  private:
    AbstractType::Ptr keepAlive;
};

AtomicIncrementer::Int DelayedTypeResolver::depth_counter = Q_BASIC_ATOMIC_INITIALIZER(0);

// bool operator==( const ExpressionEvaluationResult& left, const ExpressionEvaluationResult& right ) {
//  return left.type == right.type && left.isInstance == right.isInstance;
// }


namespace Cpp {
// bool operator==( const QList<ExpressionEvaluationResult>& left, const QList<ExpressionEvaluationResult>& right ) {
//   return true;
// }

TemplateDeclaration::TemplateDeclaration(const TemplateDeclaration& /*rhs*/) : m_parameterContext(0), m_instantiatedFrom(0), m_specializedFrom(0) {
}

TemplateDeclaration::TemplateDeclaration() : m_parameterContext(0), m_instantiatedFrom(0), m_specializedFrom(0) {
}

Declaration* TemplateDeclaration::specialize(uint specialization, TopDUContext* topContext) {
  if(specialization == 0)
    return dynamic_cast<Declaration*>(this);
  else
    return instantiate(IndexedInstantiationInformation( specialization ).information(), topContext);
} 

uint TemplateDeclaration::specialization() const {
  if(m_instantiatedWith.isValid())
    return m_instantiatedWith.index();
  else
    return 0;
}

// DeclarationId TemplateDeclaration::id() const {
//   const Declaration* thisDecl = dynamic_cast<const Declaration*>(this);
//   if(m_instantiatedFrom) {
//     DeclarationId ret = m_instantiatedFrom->id();
//     ret.setSpecialization(m_instantiatedWith.index());
//     return ret;
//   } else{
// thisDecl->topContext()->ownIndex();
//     return thisDecl->Declaration::id();
//   }
// }

TemplateDeclaration::~TemplateDeclaration()
{
  setSpecializedFrom(0);
  
  InstantiationsHash instantiations;
  {
    QMutexLocker l(&instantiationsMutex);
    instantiations = m_instantiations;

    ///Unregister at the declaration this one is instantiated from
    if( m_instantiatedFrom ) {
      InstantiationsHash::iterator it = m_instantiatedFrom->m_instantiations.find(m_instantiatedWith);
      if( it != m_instantiatedFrom->m_instantiations.end() ) {
        Q_ASSERT(*it == this);
        m_instantiatedFrom->m_instantiations.erase(it);
      }
      
      m_instantiatedFrom = 0;
    }
  }

  ///Delete all slave-declarations
  foreach( TemplateDeclaration* decl, instantiations ) {
    ///Specializations should always be anonymous, so we can delete them here.
    decl->m_instantiatedFrom = 0;
    delete decl;
  }

  QList<TemplateDeclaration*> specializations = m_specializations;
  foreach( TemplateDeclaration* specialization, specializations )
    specialization->setSpecializedFrom(0);
}

TemplateDeclaration* TemplateDeclaration::instantiatedFrom() const {
  return m_instantiatedFrom;
}

void TemplateDeclaration::setSpecializedFrom(TemplateDeclaration* other) {
  if( m_specializedFrom )
    m_specializedFrom->m_specializations.removeAll(this);

  m_specializedFrom = other;

  if( m_specializedFrom )
    m_specializedFrom->m_specializations << this;
}

TemplateDeclaration* TemplateDeclaration::specializedFrom() const {
  return m_specializedFrom;
}

void TemplateDeclaration::setInstantiatedFrom(TemplateDeclaration* from, const InstantiationInformation& instantiatedWith)
{
  if(from && from->instantiatedFrom()) {
    //Always register as instantiation of the basic template declaration
    setInstantiatedFrom(from->instantiatedFrom(), instantiatedWith);
    return;
  }
    
  Declaration* thisDecl = dynamic_cast<Declaration*>(this);
  //Change the identifier so it contains the template-parameters
  
  QMutexLocker l(&instantiationsMutex);
  if( m_instantiatedFrom ) {
    InstantiationsHash::iterator it = m_instantiatedFrom->m_instantiations.find(m_instantiatedWith);
    if( it != m_instantiatedFrom->m_instantiations.end() && *it == this )
      m_instantiatedFrom->m_instantiations.erase(it);

    m_instantiatedFrom = 0;
  }
  m_instantiatedFrom = from;
  m_instantiatedWith = instantiatedWith.indexed();
  //Only one instantiation is allowed
  if(from) {
    Q_ASSERT(from->m_instantiations.find(instantiatedWith.indexed()) == from->m_instantiations.end());
    from->m_instantiations.insert(m_instantiatedWith, this);
    Q_ASSERT(from->m_instantiations.contains(m_instantiatedWith));
  }
}

bool TemplateDeclaration::isInstantiatedFrom(const TemplateDeclaration* other) const {
    QMutexLocker l(&instantiationsMutex);
  
    InstantiationsHash::const_iterator it = other->m_instantiations.find(m_instantiatedWith);
    if( it != other->m_instantiations.end() && (*it) == this )
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

///Returns the context assigned to the given declaration that contains the template-parameters, if available. Else zero.
DUContext* getTemplateContext(Declaration* decl)
{
  DUContext* internal = decl->internalContext();
  if( !internal )
    return 0;
  foreach( DUContextPointer ctx, internal->importedParentContexts() ) {
    if( ctx )
      if( ctx->type() == DUContext::Template )
        return ctx.data();
  }
  return 0;
}

///Applies the default-parameters from basicDeclaration. This must be called AFTER basicDeclaration was instantiated, so the
///template-parameters are already replaced with their instantiated types
// InstantiationInformation applyDefaultParameters( const InstantiationInformation& information, Declaration* basicDeclaration )
// {
//   DUContext* templateCtx = getTemplateContext(basicDeclaration);
//   if( !templateCtx )
//     return information;
// 
//   if( information.templateParametersSize() >= templateCtx->localDeclarations().count() )
//     return information;
//   
//   InstantiationInformation ret(information);
//   
//   for( int a = information.templateParameters.count(); a < templateCtx->localDeclarations().count(); a++ ) {
//     TemplateParameterDeclaration* decl = dynamic_cast<TemplateParameterDeclaration*>(templateCtx->localDeclarations()[a]);
//     if( decl ) {
//       if( !decl->defaultParameter().isEmpty() ) {
//         ExpressionEvaluationResult res;
//         res.type = decl->abstractType();
// 
//         ret.templateParametersList.append(res.indexed());
//       }else{
//         //kDebug(9007) << "missing needed default template-parameter";
//       }
//     } else {
//       kDebug(9007) << "Warning: non template-parameter in template context";
//     }
//   }
//   
//   return ret;
// }

///Reads the template-parameters from the template-context of the declaration, and puts them into the identifier.
///Must be called AFTER the declaration was instantiated.
void updateIdentifierTemplateParameters( Identifier& identifier, Declaration* basicDeclaration )
{
  identifier.clearTemplateIdentifiers();
  
  DUContext* templateCtx = getTemplateContext(basicDeclaration);
  if( !templateCtx )
    return;

  for( int a = 0; a < templateCtx->localDeclarations().count(); a++ ) {
    AbstractType::Ptr type = templateCtx->localDeclarations()[a]->abstractType();
    if(type)
        identifier.appendTemplateIdentifier( type->toString() );
    else
        identifier.appendTemplateIdentifier( QualifiedIdentifier("(missing template type)") );
  }
}

///@todo prevent endless recursion when resolving base-classes!(Parent is not yet in du-chain, so a base-class that references it will cause endless recursion)
CppDUContext<KDevelop::DUContext>* instantiateDeclarationAndContext( KDevelop::DUContext* parentContext, const TopDUContext* source, KDevelop::DUContext* context, const InstantiationInformation& templateArguments, Declaration* instantiatedDeclaration, Declaration* instantiatedFrom, bool visible )
{
  StandardCppDUContext* contextCopy = 0;
  
  if( context ) {
    ///Specialize involved contexts
    Q_ASSERT(context->parentContext()); //Top-context is not allowed
    contextCopy = new StandardCppDUContext(context->url(), context->range(), parentContext, true); //We do not need to care about TopDUContext here, because a top-context can not be instantiated
    contextCopy->setSmartRange(context->smartRange(), KDevelop::DocumentRangeObject::DontOwn); //The range belongs to the original context, so flag it not to be owned by the context
    contextCopy->setType(context->type());
    contextCopy->setLocalScopeIdentifier(context->localScopeIdentifier());

    if( instantiatedDeclaration )
      instantiatedDeclaration->setInternalContext(contextCopy);
      
    ///Now the created context is already partially functional and can be used for searching(not the instantiated template-params yet though)
    
    if( context->type() == KDevelop::DUContext::Template ) {
      ///We're in the template-parameter context, exchange the template-parameters with their values.
      ///Specialize the local template-declarations
      uint currentArgument = 0;
      
    InstantiationInformation parameterInstantiationInformation;
    parameterInstantiationInformation.previousInstantiationInformation = templateArguments.indexed().index();
    
      foreach(Declaration* decl, context->localDeclarations())
      {
        TemplateDeclaration* tempDecl = dynamic_cast<TemplateDeclaration*>(decl);
        Q_ASSERT(tempDecl);
//         tempDecl->instantiate(parameterInstantiationInformation, source, true);
        
        TemplateParameterDeclaration* templateDecl = dynamic_cast<TemplateParameterDeclaration*>(decl);
        Q_ASSERT(templateDecl); //Only template-parameter declarations are allowed in template-contexts
        TemplateParameterDeclaration* declCopy = dynamic_cast<TemplateParameterDeclaration*>(decl->clone());
        Q_ASSERT(declCopy);
        TemplateDeclaration* tempCopyDecl = dynamic_cast<TemplateDeclaration*>(declCopy);
        Q_ASSERT(tempCopyDecl);

        if( currentArgument < templateArguments.templateParametersSize() && templateArguments.templateParameters()[currentArgument].result().isValid() )
        {
          declCopy->setAbstractType( templateArguments.templateParameters()[currentArgument].result().type.type() );
        } else {
          //Apply default-parameters
          //Use the already available delayed-type resolution to resolve the value/type
          if( !templateDecl->defaultParameter().isEmpty() ) {
            DelayedType::Ptr delayed( new DelayedType() );
            delayed->setIdentifier( templateDecl->defaultParameter() );
            declCopy->setAbstractType( resolveDelayedTypes( AbstractType::Ptr(delayed.data()), contextCopy, source) );
          }else{
            //Parameter missing
          }
        }
        ///We mark the declaration as a specialization, so no index is created for it within the top-context(that needs a write-lock)
        tempCopyDecl->setInstantiatedFrom(0, parameterInstantiationInformation);
        ///This inserts the copied declaration into the copied context
        declCopy->setContext(contextCopy);
        ++currentArgument;
      }
    }
    
    ///Find  the template-parameter context, and recurse into it, so we can replace the template parameters
    foreach( KDevelop::DUContextPointer importedContext,  context->importedParentContexts() )
    {
      if( !importedContext)
        continue;
        ///For functions, the Template-context is one level deeper(it is imported by the function-context) so also copy the function-context
      if( importedContext->type() == KDevelop::DUContext::Template || importedContext->type() == KDevelop::DUContext::Function )
      {
        DUContext* ctx = instantiateDeclarationAndContext( parentContext, source, importedContext.data(), templateArguments, 0, 0);
        contextCopy->addImportedParentContext( ctx, SimpleCursor(), true );
        
        if( instantiatedDeclaration && importedContext->type() == KDevelop::DUContext::Template ) {
          TemplateDeclaration* tempDecl = dynamic_cast<TemplateDeclaration*>(instantiatedDeclaration);
          if( instantiatedDeclaration )
            tempDecl->setTemplateParameterContext( ctx );
          else
            kDebug(9007) << "instantiated declaration is not a template declaration";
        }
      }
      else
      {
        //Import all other imported contexts
        contextCopy->addImportedParentContext( importedContext.data(), SimpleCursor::invalid(), true );
      }
    }

    if( instantiatedDeclaration ) {
      ///We do not need to respect forward-declarations here, because they are not allowed as base-classes.
      const CppClassType* klass = dynamic_cast<const CppClassType*>( instantiatedDeclaration->abstractType().data() );
      if( klass ) { //It could also be a function
        ///Resolve template-dependent base-classes(They can not be found in the imports-list, because their type is DelayedType and those have no context)

        foreach( const CppClassType::BaseClassInstance& base, klass->baseClasses() ) {
          const DelayedType* delayed = dynamic_cast<const DelayedType*>(base.baseClass.data());
          if( delayed ) {
            ///Resolve the delayed type, and import the context
            DelayedTypeResolver res(contextCopy, source);
            AbstractType::Ptr newType( res.exchange(delayed) );

            if( CppClassType* baseClass = dynamic_cast<CppClassType*>(newType.data()) )
            {
              if( baseClass->declaration() && baseClass->declaration()->internalContext() )
              {
                contextCopy->addImportedParentContext( baseClass->declaration()->internalContext(), SimpleCursor::invalid(), true );
              }
            } else {
              kDebug(9007) << "Resolved bad base-class";
            }
          }
        }
      }
    }

  } else {
    ///Note: this is possible, for example for template function-declarations(They do not have an internal context, because they have no compound statement), for variables, etc..
    ///Directly take their assigned template-parameter-context and specialize it. We need it at least for overload-resolution.
    TemplateDeclaration* fromTemplateDecl = dynamic_cast<TemplateDeclaration*>(instantiatedFrom);
    TemplateDeclaration* toTemplateDecl = dynamic_cast<TemplateDeclaration*>(instantiatedDeclaration);
    if( toTemplateDecl && fromTemplateDecl && fromTemplateDecl->templateParameterContext() ) {
        DUContext* ctx = instantiateDeclarationAndContext( parentContext, source, fromTemplateDecl->templateParameterContext(), templateArguments, 0, 0);
        toTemplateDecl->setTemplateParameterContext( ctx );
    }
  }

  if( contextCopy )
    contextCopy->setInstantiatedFrom(dynamic_cast<CppDUContext<DUContext>*>(context), templateArguments);
  ///Since now the context is accessible through the du-chain, so it must not be changed any more.

  if( instantiatedDeclaration && instantiatedDeclaration->abstractType() ) {

/*    if( parentContext->type() == DUContext::Template && templateArguments.previousInstantiationInformation ) {
        DUContext* fromParent = instantiatedFrom->context();
        ///This is a template parameter, replace it
        uint currentArgument = fromParent->localDeclarations().indexOf(instantiatedFrom);
        IndexedInstantiationInformation parentInfo(templateArguments.previousInstantiationInformation);
        kDebug() << "instantiating template parameter declaration" << instantiatedFrom -> toString();
        
        if( currentArgument < parentInfo.information().templateParametersSize() && parentInfo.information().templateParameters()[currentArgument].result().isValid() )
        {
            kDebug() << "with" << parentInfo.information().templateParameters()[currentArgument].result().toString();
            instantiatedDeclaration->setAbstractType( parentInfo.information().templateParameters()[currentArgument].result().type.type() );
        } else {
            kDebug() << "with default";
            TemplateParameterDeclaration * paramDecl = dynamic_cast<TemplateParameterDeclaration*>(instantiatedFrom);
            //Apply default-parameters
            //Use the already available delayed-type resolution to resolve the value/type
            if( paramDecl && !paramDecl->defaultParameter().isEmpty() ) {
            DelayedType::Ptr delayed( new DelayedType() );
            delayed->setIdentifier( paramDecl->defaultParameter() );
            instantiatedDeclaration->setAbstractType( resolveDelayedTypes( AbstractType::Ptr(delayed.data()), contextCopy, source) );
            }else{
            //Parameter missing
            }
        }
    }else{*/
        ///Resolve all involved delayed types
        
        IdentifiedType* idType = dynamic_cast<IdentifiedType*>(instantiatedDeclaration->abstractType().data());
    
        ///Use the internal context if it exists, so undefined template-arguments can be found and the DelayedType can be further delayed then.
        AbstractType::Ptr changedType = resolveDelayedTypes( instantiatedDeclaration->abstractType(), instantiatedDeclaration->internalContext() ? instantiatedDeclaration->internalContext() : parentContext, source );
    
        if( idType && idType->declaration() == instantiatedFrom ) {
        if( changedType == instantiatedDeclaration->abstractType() )
            changedType = instantiatedDeclaration->abstractType()->clone();
        
        IdentifiedType* changedIdType = dynamic_cast<IdentifiedType*>(changedType.data());
        if( changedIdType )
            changedIdType->setDeclaration(instantiatedDeclaration);
        }
    
        instantiatedDeclaration->setAbstractType( changedType );
//     }
  }
  
  if( instantiatedDeclaration ) {
    TemplateDeclaration* instantiatedTemplate = dynamic_cast<TemplateDeclaration*>(instantiatedDeclaration);

    if(instantiatedTemplate) { //Since this is also called for normal members, this does not have to be the case.
      
      ///Change the identifier to reflect the used template-arguments
      KDevelop::Identifier id = instantiatedDeclaration->identifier();
      
      updateIdentifierTemplateParameters( id, instantiatedDeclaration );

      instantiatedDeclaration->setIdentifier(id);
    }
    ///Last step, because after this, the declaration will be potentially findable
    
    if(instantiatedTemplate && dynamic_cast<TemplateDeclaration*>(instantiatedFrom))
      instantiatedTemplate->setInstantiatedFrom(dynamic_cast<TemplateDeclaration*>(instantiatedFrom), templateArguments);
    ///@todo Check the safety of this when the declaration is not anonymous!
    instantiatedDeclaration->setContext(parentContext, !visible);
  }
  
  return contextCopy;
}

DeclarationId TemplateDeclaration::id() const
{
  if(m_instantiatedFrom) {
      DeclarationId ret = m_instantiatedFrom->id();
      ret.setSpecialization(specialization());
      return ret;
  }else{
      return dynamic_cast<const Declaration*>(this)->Declaration::id();
  }
}

///@todo Use explicitly declared specializations
Declaration* TemplateDeclaration::instantiate( const InstantiationInformation& templateArguments, const TopDUContext* source, bool visible )
{
  if( m_instantiatedFrom )
    return m_instantiatedFrom->instantiate( templateArguments, source, visible );
  
  {
    QMutexLocker l(&instantiationsMutex);
   InstantiationsHash::const_iterator it;
    it = m_instantiations.find( templateArguments.indexed() );
    if( it != m_instantiations.end() ) {
      return dynamic_cast<Declaration*>(*it);
    }
  }
  
  DUContext* surroundingContext = dynamic_cast<const Declaration*>(this)->context();
  {
    //Check whether the instantiation also instantiates the parent context, and if it does, replace surroundingContext with the instantiated version
    CppDUContext<DUContext>* parent = dynamic_cast<CppDUContext<DUContext>*>(surroundingContext);
    if(parent && templateArguments.previousInstantiationInformation && templateArguments.previousInstantiationInformation != parent->instantiatedWith().index()) {
      ///An instantiation has been requested, where the parent-context is specialized.
      ///Specialize the parent
      Declaration* newParentDecl = parent->owner();
      if(newParentDecl) {
        TemplateDeclaration* templDec = dynamic_cast<TemplateDeclaration*>(newParentDecl);
        if(templDec)
          newParentDecl = templDec->instantiate( IndexedInstantiationInformation(templateArguments.previousInstantiationInformation).information(), source, visible );
      }
      
      if(newParentDecl && newParentDecl->internalContext())
        surroundingContext = newParentDecl->internalContext();
    }
  }
  
  Declaration* decl = dynamic_cast<Declaration*>(this);
  Q_ASSERT(decl);
  Declaration* clone = decl->clone();
  Q_ASSERT(clone);
  TemplateDeclaration* cloneTemplateDecl = dynamic_cast<TemplateDeclaration*>(clone);
  Q_ASSERT(cloneTemplateDecl);
  
  ///Now eventually create the virtual contexts, and fill new information into the declaration
  instantiateDeclarationAndContext( surroundingContext, source, decl->internalContext(), templateArguments, clone, decl, visible );

//  cloneTemplateDecl->setInstantiatedFrom(this);

  return clone;
}

AbstractType::Ptr resolveDelayedTypes( AbstractType::Ptr type, const KDevelop::DUContext* context, const KDevelop::TopDUContext* source, KDevelop::DUContext::SearchFlags searchFlags ) {
  if( !type )
    return type;

  ///First, find out if delayed types are involved(Function return-type, function-arguments, reference/pointer target, base-class)
  DelayedTypeSearcher search;
  
  type->accept(&search);

  DelayedType* delayedType = dynamic_cast<DelayedType*>(type.data());

  if( search.found || delayedType ) {
    ///Delayed types were found. We must copy the whole type, and replace the delayed types.

    DelayedTypeResolver resolver(context, source, searchFlags);

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

// void TemplateDeclaration::setInstantiatedWith(const InstantiationInformation& with) {
//   Q_ASSERT(!m_instantiatedFrom);
//   m_instantiatedWith = with.indexed();
// }

IndexedInstantiationInformation TemplateDeclaration::instantiatedWith() const {
  return m_instantiatedWith;
}

TemplateDeclaration::InstantiationsHash TemplateDeclaration::instantiations() const {
    QMutexLocker l(&instantiationsMutex);
    return m_instantiations;
}

template<>
Declaration* SpecialTemplateDeclaration<ForwardDeclaration>::resolve(const TopDUContext* topContext) const {
  if( instantiatedFrom() ) {
    SpecialTemplateDeclaration<ForwardDeclaration>* instantiatedFrom = dynamic_cast<SpecialTemplateDeclaration<ForwardDeclaration>*>(this->instantiatedFrom());
    if( instantiatedFrom ) {
      Declaration* baseResolved = instantiatedFrom->resolve(topContext);
      TemplateDeclaration* baseTemplate = dynamic_cast<TemplateDeclaration*>(baseResolved);
      if( baseResolved && baseTemplate ) {
        Declaration* ret = baseTemplate->instantiate(instantiatedWith().information(), topContext ? topContext : this->topContext());
        return ret;
      }else{
          //Forward-declaration was not resolved
          return 0;
      }
    }else{
      //TODO: report this in the problem reporter?
      kWarning(9007) << "Problem in template forward-declaration";
      return 0;
    }
  }else{
    return ForwardDeclaration::resolve(topContext);
  }
}

}

