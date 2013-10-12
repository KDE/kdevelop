
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

#include <QThreadStorage>
#include <kglobal.h>

#include <language/duchain/declaration.h>
#include <language/duchain/declarationdata.h>
#include <language/duchain/forwarddeclaration.h>
#include <language/duchain/aliasdeclaration.h>
#include <language/duchain/functiondeclaration.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/repositories/itemrepository.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/appendedlist.h>

#include "templateparameterdeclaration.h"
#include "qtfunctiondeclaration.h"
#include "cppducontext.h"
#include "expressionparser.h"
#include "templateresolver.h"
#include <language/duchain/classdeclaration.h>
#include <language/duchain/duchainregister.h>
#include <util/pushvalue.h>
#include <name_compiler.h>
#include <parsesession.h>
#include <rpp/chartools.h>
#include <rpp/pp-location.h>
#include <control.h>
#include <parser.h>
#include <typeinfo>

using namespace KDevelop;
using namespace Cpp;

#define REGISTER_TEMPLATE_DECLARATION(Declaration) typedef SpecialTemplateDeclaration<Declaration> TheTemplate ## Declaration; \
REGISTER_DUCHAIN_ITEM_WITH_DATA(TheTemplate ## Declaration, SpecialTemplateDeclarationData<Declaration::Data>);

REGISTER_TEMPLATE_DECLARATION(Declaration)
REGISTER_TEMPLATE_DECLARATION(ClassDeclaration)
REGISTER_TEMPLATE_DECLARATION(TemplateParameterDeclaration)
REGISTER_TEMPLATE_DECLARATION(ClassFunctionDeclaration)
REGISTER_TEMPLATE_DECLARATION(ClassMemberDeclaration)
REGISTER_TEMPLATE_DECLARATION(FunctionDeclaration)
REGISTER_TEMPLATE_DECLARATION(QtFunctionDeclaration)
REGISTER_TEMPLATE_DECLARATION(FunctionDefinition)
REGISTER_TEMPLATE_DECLARATION(AliasDeclaration)
REGISTER_TEMPLATE_DECLARATION(ForwardDeclaration)

QMutex TemplateDeclaration::instantiationsMutex(QMutex::Recursive);

typedef CppDUContext<KDevelop::DUContext> StandardCppDUContext;

namespace Cpp {
  DEFINE_LIST_MEMBER_HASH(SpecialTemplateDeclarationData, m_specializations, IndexedDeclaration)
  DEFINE_LIST_MEMBER_HASH(SpecialTemplateDeclarationData, m_specializedWith, IndexedType)
}

AbstractType::Ptr applyPointerReference( AbstractType::Ptr ptr, const KDevelop::IndexedTypeIdentifier& id ) {
  AbstractType::Ptr ret = ptr;

  if(ret && ((static_cast<bool>(ret->modifiers() & AbstractType::ConstModifier) != id.isConstant())
         || (static_cast<bool>(ret->modifiers() & AbstractType::VolatileModifier) != id.isVolatile()))) {
    ret->setModifiers((id.isConstant() ? AbstractType::ConstModifier : AbstractType::NoModifiers)
                    | (id.isVolatile() ? AbstractType::VolatileModifier : AbstractType::NoModifiers));
  }

  for( int a = 0; a < id.pointerDepth(); ++a ) {
    uint modifiers = AbstractType::NoModifiers;
    if( id.isConstPointer( a ) )
      modifiers = AbstractType::ConstModifier;

    PointerType::Ptr newRet( new PointerType() );
    newRet->setModifiers(modifiers);
    newRet->setBaseType( ret );
    ret = newRet.cast<AbstractType>();
  }

  if(id.isReference() ) {
    uint modifiers = AbstractType::NoModifiers;
    if( id.isConstant() )
      modifiers |= AbstractType::ConstModifier;
    if( id.isVolatile() )
      modifiers |= AbstractType::VolatileModifier;

    ReferenceType::Ptr newRet( new ReferenceType() );
    newRet->setModifiers(modifiers);
    newRet->setBaseType( ret );
    newRet->setIsRValue(id.isRValue());
    ret = newRet.cast<AbstractType>();
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
    return !found;
  }

  virtual bool visit (const StructureType *) {
    ///We do not want to visit member-types, so return false here
    return false;
  }
};

namespace Cpp {
TypePtr<DelayedType> containsDelayedType(AbstractType::Ptr type)
{
  PointerType::Ptr pType = type.cast<PointerType>();
  ReferenceType::Ptr rType = type.cast<ReferenceType>();
  DelayedType::Ptr delayedType = type.cast<DelayedType>();
  TypeAliasType::Ptr aType = type.cast<TypeAliasType>();
  if( pType )
    return containsDelayedType(pType->baseType());
  if( rType )
    return containsDelayedType(rType->baseType());
  if( aType )
    return containsDelayedType(aType->type());

  return delayedType;
}
}

/**
 * Thread-local data to ensure recursion limits are not exceeded.
 * Also holds the implementation data to support defaulte template parameters.
 */
struct ThreadLocalData {
  ThreadLocalData()
  : delayedDepth(0)
  , aliasDepth(0)
  {}
  // used to apply default template parameters
  QMultiHash<IndexedQualifiedIdentifier, IndexedType> typeOverloads;
  // recursion counter for delayed type resolution
  uint delayedDepth;
  // recursion counter for alias type resolution
  uint aliasDepth;
};

#if (QT_VERSION >= 0x040801)
QThreadStorage<ThreadLocalData> threadData;
inline ThreadLocalData& threadDataLocal() {
  return threadData.localData();
}
#else
QThreadStorage<ThreadLocalData*> threadData;
inline ThreadLocalData& threadDataLocal() {
  if(!threadData.localData())
    threadData.setLocalData(new ThreadLocalData());
  return *threadData.localData();
}
#endif

/**
 * RAII class to push/pop a type overload for a given identifier.
 */
struct PushTypeOverload
{
  PushTypeOverload(const IndexedQualifiedIdentifier& qid_, const IndexedType& type_)
  : qid(qid_)
  , type(type_)
  , data(threadDataLocal())
  {
    data.typeOverloads.insert(qid, type);
  }
  ~PushTypeOverload()
  {
    data.typeOverloads.remove(qid, type);
  }
private:
  IndexedQualifiedIdentifier qid;
  IndexedType type;
  ThreadLocalData& data;
};

/**
 * Replaces any DelayedTypes in interesting positions with their resolved versions,
 * if they can be resolved.
 */
struct DelayedTypeResolver : public KDevelop::TypeExchanger
{
  const KDevelop::DUContext* searchContext;
  const KDevelop::TopDUContext* source;
  KDevelop::DUContext::SearchFlags searchFlags;

  DelayedTypeResolver(const DUContext* _searchContext,
                      const TopDUContext* _source,
                      DUContext::SearchFlags _searchFlags = DUContext::NoUndefinedTemplateParams)
  : searchContext(_searchContext)
  , source(_source)
  , searchFlags(_searchFlags)
  { }

  virtual AbstractType::Ptr exchange( const AbstractType::Ptr& type )
  {
    ThreadLocalData& data = threadDataLocal();
    PushValue<uint> inc(data.delayedDepth, data.delayedDepth + 1);
    if( data.delayedDepth > 30 ) {
      kDebug(9007) << "Too much depth in DelayedTypeResolver::exchange, while exchanging" << (type ? type->toString() : QString("(null)"));
      return type;
    }
    DelayedType::Ptr delayedType = type.cast<DelayedType>();

    if( delayedType && delayedType->kind() == DelayedType::Delayed ) {
      QualifiedIdentifier qid = delayedType->identifier().identifier().identifier();
      if( !qid.isExpression() ) {
        // look for default template parameters
        IndexedType indexedType = data.typeOverloads.value(qid);
        if(!indexedType) {
          // fall back to normal DUContext search
          DUContext::SearchItem::PtrList identifiers;
          identifiers << DUContext::SearchItem::Ptr( new DUContext::SearchItem(qid) );
          DUContext::DeclarationList decls;
          if( !searchContext->findDeclarationsInternal( identifiers, searchContext->range().end, AbstractType::Ptr(), decls, source, searchFlags, 0 ) )
            return type;

          if( !decls.isEmpty() ) {
            indexedType = decls[0]->indexedType();
          }
        }

        if( indexedType.isValid() ) {
          return applyPointerReference(indexedType.abstractType(), delayedType->identifier());
        }
      }
      ///Resolution as type has failed, or is not appropriate.
      ///Resolve delayed expression, for example static numeric expressions
      ExpressionParser p;
      ExpressionEvaluationResult res;
      if( qid.isExpression() )
        res = p.evaluateExpression( delayedType->identifier().toString().toUtf8(), DUContextPointer(const_cast<DUContext*>(searchContext)), source );
      else
        res = p.evaluateType( delayedType->identifier().toString().toUtf8(), DUContextPointer(const_cast<DUContext*>(searchContext)), source );

      // NOTE: This looks hacky, but see e.g. TestDUChain::testDecltypeTypedef - we really _never_
      //       can replace a delayed type with a CppTemplateParameterType. Instead we need to create
      //       a new delayed type for that with its identifier.
      //TODO: try to add support for replacing CppTemplateParameterTypes directly eventually...
      //      dunno if its actually doable though
      AbstractType::Ptr ret = res.type.abstractType();
      if (CppTemplateParameterType::Ptr tplParam = ret.cast<CppTemplateParameterType>()) {
        DelayedType::Ptr delayed(new DelayedType);
        delayed->setIdentifier(IndexedTypeIdentifier(tplParam->declarationId().qualifiedIdentifier().last().toString()));
        return delayed.cast<AbstractType>();
      }
      return ret;
    }else{
      if( containsDelayedType(type) )
      {
        //Copy the type to keep the correct reference/pointer structure
        AbstractType::Ptr typeCopy( type->clone() );
        PointerType::Ptr pType = typeCopy.cast<PointerType>();
        ReferenceType::Ptr rType = typeCopy.cast<ReferenceType>();
        TypeAliasType::Ptr aType = typeCopy.cast<TypeAliasType>();
        if( pType ) //Replace the base
          pType->exchangeTypes(this);
        if( rType ) //Replace the base
          rType->exchangeTypes(this);
        if( aType )
          aType->exchangeTypes(this);

        return typeCopy;
      }
    }

    return type;
  }

  virtual bool exchangeMembers() const {
    return false;
  }
  private:
    AbstractType::Ptr keepAlive;
};

// bool operator==( const ExpressionEvaluationResult& left, const ExpressionEvaluationResult& right ) {
//  return left.type == right.type && left.isInstance == right.isInstance;
// }


namespace Cpp {
// bool operator==( const QList<ExpressionEvaluationResult>& left, const QList<ExpressionEvaluationResult>& right ) {
//   return true;
// }

TemplateDeclaration::TemplateDeclaration(const TemplateDeclaration& /*rhs*/)
: m_instantiatedFrom(0)
, m_instantiationDepth(0)
{
}

TemplateDeclaration::TemplateDeclaration()
: m_instantiatedFrom(0)
, m_instantiationDepth(0)
{
}

Declaration* TemplateDeclaration::specialize(const IndexedInstantiationInformation& specialization,
                                             const TopDUContext* topContext, int upDistance) {
  if(!specialization.isValid())
    return dynamic_cast<Declaration*>(this);
  else {
    InstantiationInformation information = IndexedInstantiationInformation( specialization ).information();
    
    //Add empty elements until the specified depth
    for(int a = 0; a < upDistance; ++a) {
      InstantiationInformation nextInformation;
      nextInformation.previousInstantiationInformation = information.indexed();
      information = nextInformation;
    }
    
    return instantiate(information, topContext);
  }
}

IndexedInstantiationInformation TemplateDeclaration::specialization() const {
  if(m_instantiatedWith.isValid())
    return m_instantiatedWith;
  else
    return IndexedInstantiationInformation();
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

DUContext* TemplateDeclaration::templateContext(const TopDUContext* source) const {
  return getTemplateContext(dynamic_cast<Declaration*>(const_cast<TemplateDeclaration*>(this)), source);
}

TemplateDeclaration::~TemplateDeclaration()
{
  InstantiationsHash instantiations;
  {
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

  deleteAllInstantiations();
}

TemplateDeclaration* TemplateDeclaration::instantiatedFrom() const {
  return m_instantiatedFrom;
}

void TemplateDeclaration::setSpecializedFrom(TemplateDeclaration* other) {
  
  if(other && other->instantiatedFrom()) {
    setSpecializedFrom(other->instantiatedFrom());
    return;
  }
  if(other && other->specializedFrom().data()) {
    setSpecializedFrom(dynamic_cast<TemplateDeclaration*>(other->specializedFrom().data()));
    return;
  }
  
  IndexedDeclaration indexedSelf(dynamic_cast<Declaration*>(this));
  IndexedDeclaration indexedOther(dynamic_cast<Declaration*>(other));
  Q_ASSERT(indexedSelf.data());
  
  if( TemplateDeclaration* tplDec = dynamic_cast<TemplateDeclaration*>(specializedFrom().data()) )
    tplDec->removeSpecializationInternal(indexedSelf);

  setSpecializedFromInternal(indexedOther);

  if( TemplateDeclaration* otherTemplate = dynamic_cast<TemplateDeclaration*>(indexedOther.data()) ) {
    otherTemplate->addSpecializationInternal(indexedSelf);
    otherTemplate->deleteAllInstantiations();
  }
}

void TemplateDeclaration::reserveInstantiation(const IndexedInstantiationInformation& info) {
  QMutexLocker l(&instantiationsMutex);

  Q_ASSERT(m_instantiations.find(info) == m_instantiations.end());
  m_instantiations.insert(info, 0);
}

///Reads the template-parameters from the template-context of the declaration, and puts them into the identifier.
///Must be called AFTER the declaration was instantiated.
void updateIdentifierTemplateParameters( Identifier& identifier, Declaration* basicDeclaration, const TopDUContext* top )
{
  identifier.clearTemplateIdentifiers();
  
  TemplateDeclaration* tempDecl = dynamic_cast<TemplateDeclaration*>(basicDeclaration);
  if(tempDecl) {
    InstantiationInformation specializedWith(tempDecl->specializedWith().information());
    if(specializedWith.templateParametersSize()) {
      //Use the information from the specialization-information to build the template-identifiers
      FOREACH_FUNCTION(const IndexedType& indexedType, specializedWith.templateParameters) {
        AbstractType::Ptr type = indexedType.abstractType();
        if(type)
          identifier.appendTemplateIdentifier( IndexedTypeIdentifier(type->toString()) );
        else
          identifier.appendTemplateIdentifier( IndexedTypeIdentifier("(missing template type)") );
      }
      return;
    }
  }

  DUContext* templateCtx = getTemplateContext(basicDeclaration, top);
  if( !templateCtx )
    return;

  for( int a = 0; a < templateCtx->localDeclarations().count(); a++ ) {
    AbstractType::Ptr type = templateCtx->localDeclarations()[a]->abstractType();
    if(type)
        identifier.appendTemplateIdentifier( IndexedTypeIdentifier(type->toString()) );
    else
        identifier.appendTemplateIdentifier( IndexedTypeIdentifier("(missing template type)") );
  }
}

void TemplateDeclaration::setInstantiatedFrom(TemplateDeclaration* from, const InstantiationInformation& instantiatedWith)
{
  Q_ASSERT(from != this);
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
    //Either it must be reserved, or not exist yet
    Q_ASSERT(from->m_instantiations.find(instantiatedWith.indexed()) == from->m_instantiations.end() || (*from->m_instantiations.find(instantiatedWith.indexed())) == 0);
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
  dynamicTemplateData()->m_parameterContext = context;
}

KDevelop::DUContext* TemplateDeclaration::templateParameterContext() const {
  return const_cast<KDevelop::DUContext*>(templateData()->m_parameterContext.data()); ///@todo make data() const; return non-const pointer in duchain-pointer
}

bool isTemplateDeclaration(const KDevelop::Declaration* decl) {
  return (bool)dynamic_cast<const TemplateDeclaration*>(decl);
}

///@todo prevent endless recursion when resolving base-classes!(Parent is not yet in du-chain, so a base-class that references it will cause endless recursion)
CppDUContext<KDevelop::DUContext>* instantiateDeclarationAndContext( KDevelop::DUContext* parentContext, const TopDUContext* source, KDevelop::DUContext* context, const InstantiationInformation& templateArguments, Declaration* instantiatedDeclaration, Declaration* instantiatedFrom, bool doNotRegister )
{
  Q_ASSERT(parentContext);
  TemplateDeclaration* instantiatedFromTemplate = dynamic_cast<TemplateDeclaration*>(instantiatedFrom);

  StandardCppDUContext* contextCopy = 0;

  if( context ) {
    ///Specialize involved contexts
    Q_ASSERT(context->parentContext()); //Top-context is not allowed
    contextCopy = new StandardCppDUContext(context->range(), parentContext, true); //We do not need to care about TopDUContext here, because a top-context can not be instantiated
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
    parameterInstantiationInformation.previousInstantiationInformation = templateArguments.indexed();

      foreach(Declaration* decl, context->localDeclarations())
      {
#ifdef QT_DEBUG
        TemplateDeclaration* tempDecl = dynamic_cast<TemplateDeclaration*>(decl);
        Q_ASSERT(tempDecl);
//         tempDecl->instantiate(parameterInstantiationInformation, source, true);
#endif

        TemplateParameterDeclaration* templateDecl = dynamic_cast<TemplateParameterDeclaration*>(decl);
        Q_ASSERT(templateDecl); //Only template-parameter declarations are allowed in template-contexts
        TemplateParameterDeclaration* declCopy = dynamic_cast<TemplateParameterDeclaration*>(decl->clone());
        Q_ASSERT(declCopy);
        TemplateDeclaration* tempCopyDecl = dynamic_cast<TemplateDeclaration*>(declCopy);
        Q_ASSERT(tempCopyDecl);

        if( currentArgument < templateArguments.templateParametersSize() && templateArguments.templateParameters()[currentArgument].abstractType() )
        {
          declCopy->setAbstractType( templateArguments.templateParameters()[currentArgument].abstractType() );
        } else {
          //Apply default-parameters, although these should have been applied before
          //Use the already available delayed-type resolution to resolve the value/type
          if( !templateDecl->defaultParameter().isEmpty() ) {
            DelayedType::Ptr delayed( new DelayedType() );
            delayed->setIdentifier( IndexedTypeIdentifier(templateDecl->defaultParameter()) );
            declCopy->setAbstractType( resolveDelayedTypes( delayed.cast<AbstractType>(), contextCopy, source) );
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
    foreach( const DUContext::Import &importedContext,  context->importedParentContexts() )
    {
      CppDUContext<DUContext>* import = dynamic_cast<CppDUContext<DUContext>*>(importedContext.context(source));
      if( !import)
        continue;
        ///For functions, the Template-context is one level deeper(it is imported by the function-context) so also copy the function-context
      if( import->type() == KDevelop::DUContext::Template || import->type() == KDevelop::DUContext::Function )
      {
        DUContext* ctx = import->instantiate(templateArguments, source);
        contextCopy->addImportedParentContext( ctx, CursorInRevision(), true );

        if( instantiatedDeclaration && import->type() == KDevelop::DUContext::Template ) {
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
        contextCopy->addImportedParentContext( import, CursorInRevision::invalid(), true );
      }
    }

    if( instantiatedDeclaration ) {
      ///We do not need to respect forward-declarations here, because they are not allowed as base-classes.
      ClassDeclaration* klass = dynamic_cast<ClassDeclaration*>( instantiatedDeclaration );
      if( klass ) { //It could also be a function
        ///Resolve template-dependent base-classes(They can not be found in the imports-list, because their type is DelayedType and those have no context)
        uint num = 0;
        FOREACH_FUNCTION( const BaseClassInstance& base, klass->baseClasses ) {
          DelayedType::Ptr delayed = base.baseClass.type<DelayedType>();
          if( delayed ) {
            ///Resolve the delayed type, and import the context
            DelayedTypeResolver res(contextCopy, source);
            AbstractType::Ptr newType( res.exchange(delayed.cast<AbstractType>()) );
            newType = TypeUtils::unAliasedType(newType);

            if( CppClassType::Ptr baseClass = newType.cast<CppClassType>() )
            {
              if( baseClass->declaration(source) && baseClass->declaration(source)->internalContext() )
              {
                contextCopy->addImportedParentContext( baseClass->declaration(source)->internalContext(), CursorInRevision::invalid(), true );
              }
              BaseClassInstance newInstance(base);
              newInstance.baseClass = newType->indexed();
              klass->replaceBaseClass( num, newInstance );
            } else {
              kWarning(9007) << "Resolved bad base-class" << delayed->toString() << (newType ? newType->toString() : QString());
            }
          }
          ++num;
        }
      }
    }

  } else {
    ///Note: this is possible, for example for template function-declarations(They do not have an internal context, because they have no compound statement), for variables, etc..
    ///Directly take their assigned template-parameter-context and specialize it. We need it at least for overload-resolution.
    TemplateDeclaration* fromTemplateDecl = dynamic_cast<TemplateDeclaration*>(instantiatedFrom);
    TemplateDeclaration* toTemplateDecl = dynamic_cast<TemplateDeclaration*>(instantiatedDeclaration);
    if( toTemplateDecl && fromTemplateDecl && fromTemplateDecl->templateParameterContext() ) {
        CppDUContext<DUContext>* templCtx = dynamic_cast<CppDUContext<DUContext>*>(fromTemplateDecl->templateParameterContext());
        DUContext* ctx = templCtx->instantiate(templateArguments, source);
        toTemplateDecl->setTemplateParameterContext( ctx );
    }
  }

  if( contextCopy && !doNotRegister )
    contextCopy->setInstantiatedFrom(dynamic_cast<CppDUContext<DUContext>*>(context), templateArguments);

  ///Since now the context is accessible through the du-chain, so it must not be changed any more.

  if( instantiatedDeclaration && instantiatedDeclaration->abstractType() ) {
        ///an AliasDeclaration represents a C++ "using bla::bla;" declaration.
        if(AliasDeclaration* alias = dynamic_cast<AliasDeclaration*>(instantiatedDeclaration)) {
          ThreadLocalData& data = threadDataLocal();
          PushValue<uint> safety(data.aliasDepth, data.delayedDepth + 1);
          if(data.aliasDepth > 30) {
            kWarning() << "depth-limit reached while resolving alias-declaration" << alias->identifier().toString() << "within" << parentContext->scopeIdentifier(true).toString();
          }else {
            ///For alias declaration, we resolve the declaration that is aliased instead of a type.
            ///For this reason, template alias-declarations have a DelayedType assigned
            DelayedType::Ptr delayed = alias->type<DelayedType>();
            if(delayed) {
              QList<Declaration*> declarations = parentContext->findDeclarations(delayed->identifier().identifier().identifier());
              if(!declarations.isEmpty())
                alias->setAliasedDeclaration(declarations.first());
            }
          }
        }else{
          TemplateDeclaration* instantiatedTemplate = dynamic_cast<TemplateDeclaration*>(instantiatedDeclaration);
          InstantiationInformation globalTemplateArguments = templateArguments;
          
          if(instantiatedTemplate) {
            //Update the "specializedWith" information
            InstantiationInformation oldSpecializedWith = instantiatedTemplate->specializedWith().information();
            if(oldSpecializedWith.templateParametersSize()) {
              //Replace the delayed types in the specialization-information with their real types
              InstantiationInformation newSpecializedWith(oldSpecializedWith);
              newSpecializedWith.templateParametersList().clear();
              FOREACH_FUNCTION(const IndexedType& type, oldSpecializedWith.templateParameters)
                newSpecializedWith.addTemplateParameter(resolveDelayedTypes(type.abstractType(), instantiatedDeclaration->internalContext() ? instantiatedDeclaration->internalContext() : parentContext, source ));
              instantiatedTemplate->setSpecializedWith(newSpecializedWith.indexed());
              globalTemplateArguments = newSpecializedWith;
            }
          }
          
          ///Resolve all involved delayed types
          AbstractType::Ptr t(instantiatedDeclaration->abstractType());
          IdentifiedType* idType = dynamic_cast<IdentifiedType*>(t.unsafeData());

          ///Use the internal context if it exists, so undefined template-arguments can be found and the DelayedType can be further delayed then.
          AbstractType::Ptr changedType = resolveDelayedTypes( instantiatedDeclaration->abstractType(), instantiatedDeclaration->internalContext() ? instantiatedDeclaration->internalContext() : parentContext, source );

          if( idType && idType->declarationId() == instantiatedFrom->id() ) {
            if( changedType == instantiatedDeclaration->abstractType() )
                changedType = instantiatedDeclaration->abstractType()->clone();

            IdentifiedType* changedIdType = dynamic_cast<IdentifiedType*>(changedType.unsafeData());
            if( changedIdType ) {
              
              DeclarationId base = instantiatedFrom->id();
              
              if(instantiatedFromTemplate && instantiatedFromTemplate->specializedFrom().data())
                base = instantiatedFromTemplate->specializedFrom().data()->id();
              
              base.setSpecialization(globalTemplateArguments.indexed());
              changedIdType->setDeclarationId(base);
            }
          }
          instantiatedDeclaration->setAbstractType( changedType );
        }

//     }
  }

  if( instantiatedDeclaration ) {
    TemplateDeclaration* instantiatedTemplate = dynamic_cast<TemplateDeclaration*>(instantiatedDeclaration);

    if(instantiatedTemplate && templateArguments.templateParametersSize()) { //Since this is also called for normal members, this does not have to be the case.

      ///Change the identifier to reflect the used template-arguments
      KDevelop::Identifier id = instantiatedDeclaration->identifier();

      updateIdentifierTemplateParameters( id, instantiatedDeclaration, source );

      instantiatedDeclaration->setIdentifier(id);
    }
    ///Last step, because after this, the declaration will be potentially findable

    if(instantiatedTemplate && instantiatedFromTemplate)
      instantiatedTemplate->setInstantiatedFrom(instantiatedFromTemplate, templateArguments);

    ///@todo check for possible multi-threading issues when inserting visible declarations into anonymous contexts
    instantiatedDeclaration->setContext(parentContext, templateArguments.templateParametersSize() || parentContext->inSymbolTable());
  }

  return contextCopy;
}

DeclarationId TemplateDeclaration::id(bool forceDirect) const
{
  if(m_instantiatedFrom) {
      DeclarationId ret = m_instantiatedFrom->id(forceDirect);
      ret.setSpecialization(specialization());
      return ret;
  }else{
      return dynamic_cast<const Declaration*>(this)->Declaration::id(forceDirect);
  }
}

void TemplateDeclaration::deleteAllInstantiations()
{
  if(m_instantiations.isEmpty() && m_defaultParameterInstantiations.isEmpty())
    return;

  InstantiationsHash instantiations;
  {
    QMutexLocker l(&instantiationsMutex);
    instantiations = m_instantiations;
    m_defaultParameterInstantiations.clear();
    m_instantiations.clear();
  }
  
  foreach( TemplateDeclaration* decl, instantiations ) {
    Q_ASSERT(decl);
    decl->m_instantiatedFrom = 0;
    //Only delete real insantiations, not specializations
    //FIXME: before this checked for decl->isAnonymous
    //This was a problem because some instantiations are not anonymous, so they end up orphaned from their m_instantiatedFrom
    //If strange crashes start cropping up in template code, this needs more thought
    if(!decl->specializedFrom().isValid()) {
      Declaration* realDecl = dynamic_cast<Declaration*>(decl);
      delete realDecl;
    }
  }
}

//TODO: QHash?
typedef QMap<IndexedString, AbstractType::Ptr> TemplateParams;

uint matchInstantiationParameters(const InstantiationInformation &info, const InstantiationInformation &matchAgainst,
                                  const TopDUContext *topCtxt, TemplateParams &requiredParams)
{
  TemplateResolver resolver(topCtxt);
  uint matchQuality = 1;
  for(uint a = 0; a < info.templateParametersSize(); ++a) {
    uint parameterMatchQuality = resolver.matchTemplateParameterTypes(info.templateParameters()[a].abstractType(),
                                                                      matchAgainst.templateParameters()[a].abstractType(),
                                                                      requiredParams);
    if (!parameterMatchQuality)
      return 0;
    matchQuality += parameterMatchQuality;
  }
  return matchQuality;
}

//Returns the first (and barring bugs the only) imported template context.
//A template decl imports one template context, which in turn imports the template context for the next scope, etc.
DUContext* nextTemplateContext(const DUContext* importingContext, const TopDUContext *topCtxt)
{
  foreach( const DUContext::Import &import, importingContext->importedParentContexts() ) {
    DUContext* c = import.context(topCtxt);
    if (c && c->type() == DUContext::Template)
      return c;
  }
  return 0;
}

uint TemplateDeclaration::matchInstantiation(IndexedInstantiationInformation indexedInfo, const TopDUContext* topCtxt,
                                             InstantiationInformation &instantiateWith, bool &instantiationRequired) const
{
  DUContext *templateContext = this->templateParameterContext();
  IndexedInstantiationInformation indexedSpecializedWith = this->specializedWith();
  uint matchQuality = 1;
  instantiationRequired = false;
  while(indexedInfo.isValid() && templateContext)
  {
    if (templateContext->localDeclarations().size())
      instantiationRequired = true;
    InstantiationInformation info = indexedInfo.information();
    InstantiationInformation specializedWith = indexedSpecializedWith.information();
    if (info.templateParametersSize() != specializedWith.templateParametersSize())
        return 0;
    if (!info.templateParametersSize())
    {
      indexedInfo = info.previousInstantiationInformation;
      indexedSpecializedWith = specializedWith.previousInstantiationInformation;
      continue;
    }
    TemplateParams requiredParameters;
    foreach(Declaration* parameterDecl, templateContext->localDeclarations())
      requiredParameters[parameterDecl->identifier().identifier()] = AbstractType::Ptr();
    uint match = matchInstantiationParameters(info, specializedWith, topCtxt, requiredParameters);
    if (!match)
      return 0;
    matchQuality += match;

    InstantiationInformation currentInstantiation;
    foreach( Declaration* decl, templateContext->localDeclarations() )
    {
      if( decl->abstractType().cast<CppTemplateParameterType>() )
      {
        IndexedString identifier = decl->identifier().identifier();
        if (requiredParameters[identifier])
          currentInstantiation.addTemplateParameter(requiredParameters[identifier]); //Take the type we have assigned.
        else
          return 0;
      }
      else
        currentInstantiation.addTemplateParameter(decl->abstractType()); //Use the specialized type
    }
    currentInstantiation.previousInstantiationInformation = instantiateWith.indexed();
    instantiateWith = currentInstantiation;

    indexedSpecializedWith = specializedWith.previousInstantiationInformation;
    indexedInfo = info.previousInstantiationInformation;
    templateContext = nextTemplateContext(templateContext, topCtxt);
  }
  return matchQuality;
}


TemplateDeclaration* TemplateDeclaration::instantiateSpecialization(const InstantiationInformation& info, const TopDUContext* source)
{
  TemplateDeclaration *specialization = 0;
  InstantiationInformation specializationInstantiationInfo;
  bool instantiationRequired;
  uint matchQuality = 0;
  FOREACH_FUNCTION(const IndexedDeclaration& decl, specializations) {
    //We only use visible specializations here
    if(source->recursiveImportIndices().contains(decl.indexedTopContext())) {
      TemplateDeclaration* curSpecialization = dynamic_cast<TemplateDeclaration*>(decl.data());
      if(curSpecialization) {
        InstantiationInformation possibleInstantiation;
        uint match = curSpecialization->matchInstantiation(info.indexed(), source, possibleInstantiation, instantiationRequired);
        if(match > matchQuality) {
          matchQuality = match;
          specializationInstantiationInfo = possibleInstantiation;
          specialization = curSpecialization;
        }
      }
    }
  }

  if (specialization) {
    if (!instantiationRequired)
      return specialization;
    else
      return dynamic_cast<TemplateDeclaration*>(specialization->instantiate(specializationInstantiationInfo, source, true));
  }
  
  return 0;
}

void applyDefaultParameters(const DUContext* templateContext, const TopDUContext* source,
                            const DUContext* surroundingContext,
                            InstantiationInformation* templateArguments)
{
  Q_ASSERT(templateContext);
  Q_ASSERT(templateContext->type() == DUContext::Template);
  Q_ASSERT(source);
  Q_ASSERT(surroundingContext);

  const int totalParameters = templateContext->localDeclarations().count();
  KDevVarLengthArray<IndexedType, 10> explicitParameters = templateArguments->templateParametersList();

  if(totalParameters <= explicitParameters.size()
     //TODO: why is this required?
     && (explicitParameters.size() != 1 || explicitParameters.at(0).isValid()))
  {
    // nothing to do
    return;
  }

  KDevVarLengthArray<IndexedType, 10> appliedParameters;
  int currentArgument = 0;

  QVector<PushTypeOverload*> typeOverloads;
  foreach(Declaration* decl, templateContext->localDeclarations()) {
    TemplateParameterDeclaration* templateDecl = dynamic_cast<TemplateParameterDeclaration*>(decl);
    Q_ASSERT(templateDecl); //Only template-parameter declarations are allowed in template-contexts

    IndexedType type = decl->indexedType();
    Q_ASSERT(type.isValid());
    if( currentArgument < explicitParameters.size()
        && explicitParameters.at(currentArgument).isValid() )
    {
      // use explicit parameter
      type = explicitParameters.at(currentArgument);
      Q_ASSERT(type);
    } else if(templateDecl->hasDefaultParameter()) {
      // Apply default-parameter
      Q_ASSERT(!templateDecl->defaultParameter().isEmpty());
      DelayedType::Ptr delayed( new DelayedType() );
      delayed->setIdentifier( IndexedTypeIdentifier(templateDecl->defaultParameter()) );
      type = resolveDelayedTypes( delayed.cast<AbstractType>(), surroundingContext, source)->indexed();
    } // else the parameter is missing

    //TODO: why is this neccessary?
    if(type.abstractType().cast<CppTemplateParameterType>()) {
      ++currentArgument;
      continue;
    }

    appliedParameters << type;
    if(type != decl->indexedType()) {
      // add type overload
      typeOverloads << new PushTypeOverload(decl->qualifiedIdentifier(), type);
    }
    ++currentArgument;
  }

  qDeleteAll(typeOverloads);
  templateArguments->templateParametersList() = appliedParameters;
}

Declaration* TemplateDeclaration::instantiate( const InstantiationInformation& _templateArguments, const TopDUContext* source, bool forceLocal )
{
  InstantiationInformation templateArguments(_templateArguments);
/*  if(dynamic_cast<TopDUContext*>(dynamic_cast<const Declaration*>(this)->context())) {
    Q_ASSERT(templateArguments.previousInstantiationInformation == 0);
  }*/
  if( m_instantiatedFrom && !forceLocal)
    return m_instantiatedFrom->instantiate( templateArguments, source );

  if ( specializedFrom().data() && !forceLocal )
    return dynamic_cast<TemplateDeclaration*>(specializedFrom().declaration())->instantiate(templateArguments, source);

  {
    QMutexLocker l(&instantiationsMutex);
    {
      DefaultParameterInstantiationHash::const_iterator it = m_defaultParameterInstantiations.constFind(templateArguments.indexed());
      if(it != m_defaultParameterInstantiations.constEnd())
        templateArguments = (*it).information();
    }
  
    InstantiationsHash::const_iterator it;
    it = m_instantiations.constFind( templateArguments.indexed() );
    if( it != m_instantiations.constEnd() ) {
      if(*it) {
        return dynamic_cast<Declaration*>(*it);
      }else{
        ///@todo What if the same thing is instantiated twice in parralel? Then this may trigger as well, altough one side should wait
        ///We are currently instantiating this declaration with the same template arguments. This would lead to an assertion.
        kDebug() << "tried to recursively instantiate" << dynamic_cast<Declaration*>(this)->toString() << "with" << templateArguments.toString();
        ///Maybe problematic, because the returned declaration is not in the correct context etc.
        return 0;
      }
    }
  }
  
  if(!source)
    return 0;
  
  if (m_instantiationDepth > 5) {
      kWarning() << "depth-limit reached while instantiating template declaration with" << _templateArguments.toString();
      return 0;
  }
  PushValue<int> depthCounter(m_instantiationDepth, m_instantiationDepth + 1);

  DUContext* surroundingContext = dynamic_cast<const Declaration*>(this)->context();
  if(!surroundingContext) {
    kDebug() << "Declaration has no context:" << dynamic_cast<Declaration*>(this)->qualifiedIdentifier().toString() << dynamic_cast<Declaration*>(this)->toString();
    return dynamic_cast<Declaration*>(this);
  }  
  
  Declaration* decl = dynamic_cast<Declaration*>(this);
  
  Q_ASSERT(decl);
  Q_ASSERT(decl->topContext());
  
  
  DUContext* templateContext = getTemplateContext(dynamic_cast<Declaration*>(this), source);
  
//   kDebug() << decl->qualifiedIdentifier().toString() << "got template-context" << templateContext << templateArguments.toString();

  if(!forceLocal) {
    if(templateContext) {
      applyDefaultParameters(templateContext, source, surroundingContext, &templateArguments);
    }

    ///Check whether there is type-aliases in the parameters that need to be resolved
    ///Generally, resolve all type-aliases that are part of a template-class, and keep the others
    {
      InstantiationInformation newTemplateArguments = templateArguments;
      
      newTemplateArguments.templateParametersList().clear();

      struct UnAliasExchanger : public KDevelop::TypeExchanger {
        UnAliasExchanger(const TopDUContext* _source) : source(_source) {
        }
        
        const TopDUContext* source;
        
        virtual KDevelop::AbstractType::Ptr exchange(const KDevelop::AbstractType::Ptr& type) {

          KDevelop::AbstractType::Ptr check = type;
          
          KDevelop::TypeAliasType::Ptr alias = type.cast<KDevelop::TypeAliasType>();
          if(alias) {
            //We exchange type-aliases with their real types only of the type-alias is in a template
            //class. In that case, we cannot be sure that it's not used for meta-programming.
            //All other aliases can be kept, for user-friendliness, even if it's not 100% C++ correct
            Declaration* decl = alias->declaration(source);
            
            if(!decl || dynamic_cast<TemplateDeclaration*>(decl)) {
              return exchange(alias->type());
            }
          }
          
          if(check)
            check->exchangeTypes(this);
          
          return check;
        }
      };
    
      UnAliasExchanger exchanger(source);
      
      for(uint a = 0; a < templateArguments.templateParametersSize(); ++a)
        newTemplateArguments.templateParametersList().append(exchanger.exchange(templateArguments.templateParameters()[a].abstractType())->indexed());
      
      templateArguments = newTemplateArguments;
    }
    
    if(!(templateArguments == _templateArguments)) {
      QMutexLocker l(&instantiationsMutex);
      m_defaultParameterInstantiations[_templateArguments.indexed()] = templateArguments.indexed();
    }
  }

  {
    //Now we have the final template-parameters. Once again check whether we have already instantiated this,
    //and if not, reserve the instantiation so we cannot crash later on
    ///@todo When the same declaration is instantuated multiple times, this sucks because one is returned invalid
    QMutexLocker l(&instantiationsMutex);
    InstantiationsHash::const_iterator it;
    it = m_instantiations.constFind( templateArguments.indexed() );
    if( it != m_instantiations.constEnd() ) {
      if(*it) {
        return dynamic_cast<Declaration*>(*it);
      }else{
        //Problem
        return dynamic_cast<Declaration*>(this);
      }
    }
    ///@warning Once we've called reserveInstantiation, we have to be 100% sure that we actually create the instantiation
    reserveInstantiation(templateArguments.indexed());
  }

  TemplateDeclaration *instantiatedSpecialization = instantiateSpecialization(templateArguments, source);

  //We have reserved the instantiation, so it must have stayed untouched
  Q_ASSERT(m_instantiations[templateArguments.indexed()] == 0);
  
  if(instantiatedSpecialization) {
    //A specialization has been chosen and instantiated. Just register it here, and return it.
    instantiatedSpecialization->setInstantiatedFrom(this, templateArguments);
    return dynamic_cast<Declaration*>(instantiatedSpecialization);
  }

  {
    //Check whether the instantiation also instantiates the parent context, and if it does, replace surroundingContext with the instantiated version
    CppDUContext<DUContext>* parent = dynamic_cast<CppDUContext<DUContext>*>(surroundingContext);
    if(parent && templateArguments.previousInstantiationInformation.index() && templateArguments.previousInstantiationInformation.index() != parent->instantiatedWith().index()) {
      DUContext* surroundingCandidate = parent->instantiate(IndexedInstantiationInformation(templateArguments.previousInstantiationInformation).information(), source);
      if(surroundingCandidate)
        surroundingContext = surroundingCandidate;
      else
        kDebug() << "could not instantiate surrounding context for" << dynamic_cast<Declaration*>(this)->qualifiedIdentifier().toString();
    }
  }

  Declaration* clone = decl->clone();
  Q_ASSERT(clone);

#ifdef QT_DEBUG
  TemplateDeclaration* cloneTemplateDecl = dynamic_cast<TemplateDeclaration*>(clone);
  Q_ASSERT(cloneTemplateDecl);
#endif

  ///Now eventually create the virtual contexts, and fill new information into the declaration
  instantiateDeclarationAndContext( surroundingContext, source, decl->internalContext(), templateArguments, clone, decl );

//  cloneTemplateDecl->setInstantiatedFrom(this);
  Q_ASSERT(clone->topContext());

  return clone;
}

AbstractType::Ptr resolveDelayedTypes( AbstractType::Ptr type, const KDevelop::DUContext* context, const KDevelop::TopDUContext* source, KDevelop::DUContext::SearchFlags searchFlags ) {
  if( !type )
    return type;

  ///First, find out if delayed types are involved(Function return-type, function-arguments, reference/pointer target, base-class)
  DelayedTypeSearcher search;

  type->accept(&search);

  DelayedType::Ptr delayedType = type.cast<DelayedType>();

  if( search.found || delayedType ) {
    ///Delayed types were found. We must copy the whole type, and replace the delayed types.

    DelayedTypeResolver resolver(context, source, searchFlags);

    AbstractType::Ptr typeCopy;
    if( delayedType )
      ///The type itself is a delayed type, resolve it
      typeCopy = resolver.exchange( type );
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
        return baseTemplate->instantiate(instantiatedWith().information(), topContext ? topContext : this->topContext());
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

