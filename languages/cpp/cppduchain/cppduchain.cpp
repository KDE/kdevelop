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

#include "cppduchain.h"
#include <language/duchain/ducontext.h>
#include <language/duchain/identifier.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainpointer.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/specializationstore.h>
#include "environmentmanager.h"
#include "parser/rpp/pp-engine.h"
#include "parser/rpp/preprocessor.h"
#include "parser/rpp/pp-environment.h"
#include "parser/rpp/pp-macro.h"
#include "parser/rpp/macrorepository.h"
#include "cppduchainexport.h"
#include <parser/rpp/chartools.h>
#include "templatedeclaration.h"
#include "cppducontext.h"
#include <language/duchain/use.h>
#include "templateparameterdeclaration.h"
#include <language/duchain/classdeclaration.h>


using namespace Cpp;
using namespace KDevelop;

const uint maxParentDepth = 20;

namespace Cpp {

KDEVCPPDUCHAIN_EXPORT  QList<KDevelop::Declaration*> findLocalDeclarations( KDevelop::DUContext* context, const KDevelop::Identifier& identifier, const TopDUContext* topContext, uint depth ) {
  QList<Declaration*> ret;

  if(depth > maxParentDepth) {
    kDebug() << "maximum parent depth reached on" << context->scopeIdentifier(true);
    return ret;
  }
  
  ret += context->findLocalDeclarations( identifier, CursorInRevision::invalid(), topContext );
  if( !ret.isEmpty() )
    return ret;

  if( context->type() != DUContext::Class )
    return ret;
  
  QVector<DUContext::Import> bases = context->importedParentContexts();
  for( QVector<DUContext::Import>::const_iterator it = bases.constBegin(); it != bases.constEnd(); ++it ) {
    if( it->context(topContext) )
      ret += findLocalDeclarations( (*it).context(topContext), identifier, topContext, depth+1 );
  }
  return ret;
}

void minimize(int& i, const int with) {
  if( with < i )
    i = with;
}

uint buildIdentifierForType(AbstractType::Ptr type, IndexedTypeIdentifier& id, uint pointerLevel, TopDUContext* top)
{
  if(!type)
    return pointerLevel;
  TypePtr< ReferenceType > refType = type.cast<ReferenceType>();
  if(refType) {
    id.setIsReference(true);
    if(refType->modifiers() & AbstractType::ConstModifier)
      id.setIsConstant(true);
    
    return buildIdentifierForType(refType->baseType(), id, pointerLevel, top);
  }
  TypePtr< PointerType > pointerType = type.cast<PointerType>();
  
  if(pointerType) {
    ++pointerLevel;
    uint maxPointerLevel = buildIdentifierForType(pointerType->baseType(), id, pointerLevel, top);
    if(type->modifiers() & AbstractType::ConstModifier)
      id.setIsConstPointer(maxPointerLevel - pointerLevel, true);
    if(static_cast<uint>(id.pointerDepth()) < pointerLevel)
      id.setPointerDepth(pointerLevel);
    
    return maxPointerLevel;
  }
  
  IdentifiedType* idType = dynamic_cast<IdentifiedType*>(type.unsafeData());
  if(idType) {
    Declaration* decl = idType->declaration(top);
    if(decl) {
      id.setIdentifier(decl->qualifiedIdentifier());
    }else
      id.setIdentifier(idType->qualifiedIdentifier());
  }else{
    //Just create it as an expression
    AbstractType::Ptr useTypeText = type;
    if(type->modifiers() & AbstractType::ConstModifier)
    {
      //Remove the 'const' modifier, as it will be added to the type-identifier below
      useTypeText = type->indexed().abstractType();
      useTypeText->setModifiers(useTypeText->modifiers() & (~AbstractType::ConstModifier));
    }
    id.setIdentifier(QualifiedIdentifier(useTypeText->toString(), true));
  }
  if(type->modifiers() & AbstractType::ConstModifier)
    id.setIsConstant(true);
  if(type->modifiers() & AbstractType::VolatileModifier)
    id.setIsVolatile(true);
  return pointerLevel;
}

IndexedTypeIdentifier identifierForType(AbstractType::Ptr type, TopDUContext* top)
{
  IndexedTypeIdentifier ret;
  buildIdentifierForType(type, ret, 0, top);
  return ret;
}

QList< QPair<Declaration*, int> > hideOverloadedDeclarations( const QList< QPair<Declaration*, int> >& declarations,
                                                              bool preferConst )
{
  QHash<Identifier, Declaration*> nearestDeclaration;
  QHash<Declaration*, int> depthHash;
  QSet<Identifier> hadNonForwardDeclaration; //Set of all non function-declarations that had a declaration that was not a forward-declaration.
  // Set of function-declarations with a const overload
  QSet<IndexedIdentifier> hasConstOverload;

  typedef QPair<Declaration*, int> Pair;
  foreach(  const Pair& decl, declarations ) {
    depthHash[decl.first] = decl.second;

    QHash<Identifier, Declaration*>::iterator it = nearestDeclaration.find(decl.first->identifier());

    bool prefer = false;
    if(it == nearestDeclaration.end()) {
      prefer = true;
    } else if((!decl.first->isForwardDeclaration() && (*it)->isForwardDeclaration())) {
      //Always prefer non forward-declarations over forward-declarations
      prefer = true;
    } else if(decl.first->isForwardDeclaration() == (*it)->isForwardDeclaration()) {
      const int depth = depthHash.value(*it);
      if (decl.second < depth) {
        prefer = true;
      } else if (decl.second == depth && decl.first->isFunctionDeclaration()
                && decl.first->context()->type() == DUContext::Class
                && TypeUtils::isConstant((*it)->abstractType()) != TypeUtils::isConstant(decl.first->abstractType()))
      {
        // const overload
        hasConstOverload << decl.first->indexedIdentifier();
      }
    }

    if(prefer) {
      nearestDeclaration[ decl.first->identifier() ] = decl.first;
    }

    if(!decl.first->isForwardDeclaration() && !decl.first->isFunctionDeclaration())
      hadNonForwardDeclaration.insert(decl.first->identifier());
  }

  QList< QPair<Declaration*, int> > ret;
    
  ///Only keep the declarations of each name on the lowest inheritance-level, or that are not forward-declarations
  foreach( const Pair& decl, declarations ) {
    if( depthHash[nearestDeclaration[decl.first->identifier()]] == decl.second ) {
      if(decl.first->isFunctionDeclaration() && decl.first->context()->type() == DUContext::Class)
      {
        // hide non-const methods if preferConst is set, or vice-versa
        const bool isConst = TypeUtils::isConstant(decl.first->abstractType());
        const bool hasOverload = hasConstOverload.contains(decl.first->indexedIdentifier());
        if (hasOverload && isConst != preferConst) {
          // if we have an overload, always prefer the version with correct constness
          continue;
        } else if (preferConst && !isConst) {
          // otherwise only hide non-cost methods if we are looking for const methods
          // const-methods should be shown when we are looking for non-const methods though
          continue;
        }
      }
      if(decl.first->isFunctionDeclaration() || !decl.first->isForwardDeclaration() || !hadNonForwardDeclaration.contains(decl.first->identifier()))
        ret << decl;
    }
  }

  return ret;
}

QList<KDevelop::Declaration*> findDeclarationsSameLevel(KDevelop::DUContext* context, const Identifier& identifier, const KDevelop::CursorInRevision& position)
{
  if( context->type() == DUContext::Namespace || context->type() == DUContext::Global ) {
    ///May have been forward-declared anywhere
    QualifiedIdentifier totalId = context->scopeIdentifier();
    totalId += identifier;
    return context->findDeclarations(totalId, position);
  }else{
    ///Only search locally within this context
    return context->findLocalDeclarations(identifier, position);
  }
}

KDevelop::QualifiedIdentifier namespaceScopeComponentFromContext(QualifiedIdentifier prefix, const KDevelop::DUContext* context, const KDevelop::TopDUContext* source)
{
  const DUContext* classContext = 0;
  
  if(context->type() == DUContext::Helper) {
    //This is a prefix-context for an external class-definition like "class A::B {..};"
    if(context->importedParentContexts().size())
      classContext = context->importedParentContexts()[0].context(source);
  } else if(context->type() == DUContext::Class) {
    classContext = context;
  }else if(context->type() == DUContext::Namespace) {
    return context->scopeIdentifier(true);
  }else{
    //This must be a function-definition, like void A::B::test() {}
    Declaration* classDeclaration = Cpp::localClassFromCodeContext(const_cast<DUContext*>(context));
    if(classDeclaration)
      classContext = classDeclaration->logicalInternalContext(source);
    if(!prefix.isEmpty())
      prefix.pop();
  }
  
  if(classContext) {
    while(!prefix.isEmpty() && classContext && classContext->type() == DUContext::Class) {
      Q_ASSERT(!prefix.isEmpty());
      prefix.pop();
      
      //This way we can correctly resolve the namespace-component for multiple externally defined classes,
      //see testDeclareStructInNamespace() in test_duchain.cpp
      if(classContext->parentContext() && classContext->parentContext()->type() == DUContext::Helper && !context->importedParentContexts().isEmpty()) {
        classContext = context->importedParentContexts()[0].context(source);
        continue;
      }
      
      break;
    }
  }
  
  return prefix;
}

Declaration* localClassFromCodeContext(const DUContext* context)
{
  if(!context)
    return 0;
  
  while( context->parentContext() && context->type() == DUContext::Other && context->parentContext()->type() == DUContext::Other )
  { //Move context to the top context of type "Other". This is needed because every compound-statement creates a new sub-context.
    context = context->parentContext();
  }
  
  if(context->type() == DUContext::Class)
    return context->owner();
  
  //For function declarations, this is the solution.
  if(context->parentContext() && context->parentContext()->type() == DUContext::Class)
    return context->parentContext()->owner();
  
  if(context->type() == DUContext::Other) {
    //Jump from code-context to function-context
    foreach(const DUContext::Import& import, context->importedParentContexts()) {
      if(DUContext* i = import.context(context->topContext())) {
        if(i->type() == DUContext::Function) {
          context = i;
          break;
        }
      }
    }
  }
  
  //For external function definitions, find the class-context by following the import-structure
  if(context->type() == DUContext::Function) {
    foreach(const DUContext::Import& import, context->importedParentContexts()) {
      DUContext* ctx = import.context(context->topContext());
      if(ctx && ctx->type() == DUContext::Class && ctx->owner())
        return ctx->owner();
    }
    
    if(!context->importers().isEmpty())
      context = context->importers().first();
  }

  return 0;
}

Declaration* localFunctionFromCodeContext(const DUContext* context)
{
  while( context->parentContext() && context->type() == DUContext::Other && context->parentContext()->type() == DUContext::Other )
  { //Move context to the top context of type "Other". This is needed because every compound-statement creates a new sub-context.
    context = context->parentContext();
  }

  if(context->type() == DUContext::Function || context->owner()) {
    return context->owner();
  }

  if(context->type() == DUContext::Other) {
    //Jump from code-context to function-context
    foreach(const DUContext::Import& import, context->importedParentContexts()) {
      if(DUContext* i = import.context(context->topContext())) {
        if(i->type() == DUContext::Function) {
          return i->owner();
        }
      }
    }
  }

  return 0;
}

ClassMemberDeclaration::AccessPolicy mostRestrictiveInheritanceAccessPolicy(DUContext* startContext, DUContext* targetContext, TopDUContext* top, bool ignoreFirst = false)
{
  ClassMemberDeclaration::AccessPolicy ret = ClassMemberDeclaration::Public;
  if(startContext != targetContext) {
    ClassDeclaration* classDecl = dynamic_cast<ClassDeclaration*>(startContext->owner());
    if(classDecl) {
      FOREACH_FUNCTION(const BaseClassInstance& import, classDecl->baseClasses) {
        AbstractType::Ptr type = import.baseClass.abstractType();
        IdentifiedType* identified = dynamic_cast<IdentifiedType*>(type.unsafeData());
        if(identified) {
          Declaration* decl = identified->declaration(top);
          ///@todo This is not very efficient
          if(decl && decl->internalContext() && decl->internalContext()->imports(targetContext)) {
            ret = mostRestrictiveInheritanceAccessPolicy(decl->internalContext(), targetContext, top);
            if(import.access > ret && !ignoreFirst)
              ret = import.access;
            break;
          }
        }
      }
    }
  }
  return ret;
}

KDEVCPPDUCHAIN_EXPORT bool isAccessible(DUContext* fromContext, ClassMemberDeclaration* declaration, TopDUContext* source, DUContext* declarationContext)
{
  if(declarationContext) {
    ClassMemberDeclaration::AccessPolicy restriction = mostRestrictiveInheritanceAccessPolicy(declarationContext, declaration->context(), source);
    if(restriction != ClassMemberDeclaration::Public)
      return false;
  }else if(!declarationContext && fromContext->type() == DUContext::Class && fromContext->imports(declaration->context())) {
    declarationContext = fromContext;
    ClassMemberDeclaration::AccessPolicy restriction = mostRestrictiveInheritanceAccessPolicy(declarationContext, declaration->context(), source, true);
    if(restriction == ClassMemberDeclaration::Private)
      return false;
  }

  ClassMemberDeclaration::AccessPolicy effectiveAccessPolicy = declaration->accessPolicy();
  
  if(effectiveAccessPolicy == ClassMemberDeclaration::Public)
    return true;
  
  if(!fromContext)
    return false;
  
  if(fromContext->type() == DUContext::Other || fromContext->type() == DUContext::Function) {
    Declaration* classDecl = localClassFromCodeContext(fromContext);
    if(!classDecl || !classDecl->internalContext()) {
      return false;
    }
    
    return isAccessible(classDecl->internalContext(), declaration, source, declarationContext);
  }
  
  if(fromContext->type() != DUContext::Class)
    return false;
  
  if(effectiveAccessPolicy == ClassMemberDeclaration::Protected) {
    if(fromContext->imports( declaration->context() )) {
      return true;
    }
  }else if(effectiveAccessPolicy == ClassMemberDeclaration::Private) {
    if(fromContext == declaration->context())
      return true;
  }
  
  if(isFriend(declaration->context()->owner(), fromContext->owner()))
    return true;
  
  DUContext* parent = logicalParentContext(fromContext, fromContext->topContext());
  
  if(parent && parent->type() == DUContext::Class)
    return isAccessible(parent, declaration, source, declarationContext);
  
  return false;
}

KDevelop::DUContext* logicalParentContext(KDevelop::DUContext* context, KDevelop::TopDUContext* source)
{
  if(!context->parentContext())
    return 0;
  
  if(context->parentContext()->type() == DUContext::Helper && !context->parentContext()->importedParentContexts().isEmpty())
    return context->parentContext()->importedParentContexts()[0].context(source);
   
  return context->parentContext();
}

/**
 * Preprocess the given string using the macros from given EnvironmentFile up to the given line
 * If line is -1, all macros are respected.
 * This is a quite slow operation, because thousands of macros need to be shuffled around.
 * */
QString preprocess( const QString& text, Cpp::EnvironmentFile* file, int line, QSet<IndexedString> disableMacros ) {

  rpp::Preprocessor preprocessor;
  rpp::pp pp(&preprocessor);

  {
      DUChainReadLocker lock(DUChain::lock());
    //Copy in all macros from the file
    for( Cpp::ReferenceCountedMacroSet::Iterator it( file->definedMacros().iterator() ); it; ++it ) {
      if( line == -1 || line > it.ref().sourceLine || file->url() != it.ref().file ) {
        if(!disableMacros.contains( it.ref().name ))
        {
          pp.environment()->setMacro( new rpp::pp_macro(it.ref()) );
        }
      }
    }
    for( Cpp::ReferenceCountedMacroSet::Iterator it( file->usedMacros().iterator() ); it; ++it ) {
      if( line == -1 || line > it.ref().sourceLine || file->url() != it.ref().file ) {
        if(!disableMacros.contains( it.ref().name ))
          pp.environment()->setMacro( new rpp::pp_macro(it.ref()) );
      }
    }
  }

  QString ret = QString::fromUtf8(stringFromContents(pp.processFile("anonymous", text.toUtf8())));
  
  return ret;
}

QPair<KDevelop::Identifier, QByteArray> qtFunctionSignature(QByteArray fullFunction) {
  
  if(fullFunction.startsWith('"') && fullFunction.endsWith('"'))
    fullFunction = fullFunction.mid(1, fullFunction.length()-2);
  
  int parenBegin = fullFunction.indexOf('(');
  int parenEnd = fullFunction.lastIndexOf(')');
  Identifier id;
  QByteArray signature;
  if(parenBegin < parenEnd && parenBegin != -1) {
    id = Identifier(IndexedString(fullFunction.left(parenBegin).trimmed()));
    signature = QMetaObject::normalizedSignature(fullFunction.mid(parenBegin, parenEnd-parenBegin+1).data());
    signature = signature.mid(1, signature.length()-2);
  }
  
  return qMakePair(id, signature);
}

KDevelop::Identifier exchangeQualifiedIdentifier(KDevelop::Identifier id, KDevelop::QualifiedIdentifier replace, KDevelop::QualifiedIdentifier replaceWith) {
  KDevelop::Identifier ret(id);
  ret.clearTemplateIdentifiers();
  for(unsigned int a = 0; a < id.templateIdentifiersCount(); ++a)
    ret.appendTemplateIdentifier(exchangeQualifiedIdentifier(id.templateIdentifier(a), replace, replaceWith));

  return ret;
}

KDevelop::IndexedTypeIdentifier exchangeQualifiedIdentifier(KDevelop::IndexedTypeIdentifier id, KDevelop::QualifiedIdentifier replace, KDevelop::QualifiedIdentifier replaceWith) {
  KDevelop::IndexedTypeIdentifier ret(id);
  QualifiedIdentifier oldId(id.identifier().identifier());
  
  QualifiedIdentifier qid;

  if(oldId == replace) {
    for(int a = 0; a < replaceWith.count(); ++a)
      qid.push(replaceWith.at(a));
  }else{
    for(int a = 0; a < oldId.count(); ++a)
      qid.push(exchangeQualifiedIdentifier(oldId.at(a), replace, replaceWith));
  }
  ret.setIdentifier(IndexedQualifiedIdentifier(qid));
  return ret;
}

KDevelop::IndexedTypeIdentifier unTypedefType(Declaration* decl, KDevelop::IndexedTypeIdentifier type) {
  for(int a = 0; a < decl->context()->usesCount(); ++a) {
    Use use = decl->context()->uses()[a];
    if(use.m_range.end > decl->range().start)
      break;
    Declaration* usedDecl = use.usedDeclaration(decl->topContext());
    ///@todo Make this work nicely for template-parameters. We need to know from where they were instantiated to do this though.
    if(usedDecl && usedDecl->isTypeAlias() && !dynamic_cast<TemplateParameterDeclaration*>(usedDecl) && TypeUtils::targetType(usedDecl->abstractType(), 0)) {
      QualifiedIdentifier exchange(TypeUtils::targetType(usedDecl->abstractType(), 0)->toString());
      QualifiedIdentifier exchangeWith(usedDecl->qualifiedIdentifier());
      type = exchangeQualifiedIdentifier(type, exchange, exchangeWith);
    }
  }
  return type;
}

IndexedTypeIdentifier removeTemplateParameters(IndexedTypeIdentifier identifier, int behindPosition);

Identifier removeTemplateParameters(Identifier id, int behindPosition) {
  Identifier ret(id);
  
  ret.clearTemplateIdentifiers();
  for(unsigned int a = 0; a < id.templateIdentifiersCount(); ++a) {
    IndexedTypeIdentifier replacement = removeTemplateParameters(id.templateIdentifier(a), behindPosition);
    if((int) a < behindPosition)
      ret.appendTemplateIdentifier(replacement);
    else {
      ret.appendTemplateIdentifier(IndexedTypeIdentifier(QualifiedIdentifier("...")));
      break;
    }
  }
  return ret;
}

IndexedTypeIdentifier removeTemplateParameters(IndexedTypeIdentifier identifier, int behindPosition) {
  IndexedTypeIdentifier ret(identifier);
  
  QualifiedIdentifier oldId(identifier.identifier().identifier());
  QualifiedIdentifier qid;
  
  for(int a = 0; a < oldId.count(); ++a)
    qid.push(removeTemplateParameters(oldId.at(a), behindPosition));

  ret.setIdentifier(qid);

  return ret;
}

KDevelop::IndexedTypeIdentifier stripPrefixIdentifiers(KDevelop::IndexedTypeIdentifier id, KDevelop::QualifiedIdentifier strip);

KDevelop::Identifier stripPrefixIdentifiers(KDevelop::Identifier id, KDevelop::QualifiedIdentifier strip) {
  KDevelop::Identifier ret(id);
  ret.clearTemplateIdentifiers();
  for(unsigned int a = 0; a < id.templateIdentifiersCount(); ++a)
    ret.appendTemplateIdentifier(stripPrefixIdentifiers(id.templateIdentifier(a), strip));

  return ret;
}

KDevelop::IndexedTypeIdentifier stripPrefixIdentifiers(KDevelop::IndexedTypeIdentifier id, KDevelop::QualifiedIdentifier strip) {

  QualifiedIdentifier oldId(id.identifier().identifier());
  QualifiedIdentifier qid;
  
  int commonPrefix = 0;
  for(;commonPrefix < oldId.count()-1 && commonPrefix < strip.count(); ++commonPrefix)
    if(strip.at(commonPrefix).toString() != oldId.at(commonPrefix).toString())
      break;
  
  for(int a = commonPrefix; a < oldId.count(); ++a)
    qid.push( stripPrefixIdentifiers(oldId.at(a), strip) );

  KDevelop::IndexedTypeIdentifier ret(id);
  ret.setIdentifier(qid);
  return ret;
}

int reservedIdentifierCount(QString name) {
  QStringList l = name.split("::");
  int ret = 0;
  foreach(const QString& s, l)
    if(s.startsWith('_'))
      ++ret;
    
  return ret;
}

AbstractType::Ptr shortenTypeForViewing(AbstractType::Ptr type) {
  struct ShortenAliasExchanger : public KDevelop::TypeExchanger {
    virtual KDevelop::AbstractType::Ptr exchange(const KDevelop::AbstractType::Ptr& type) {
      if(!type)
        return type;
      
      KDevelop::AbstractType::Ptr newType( type->clone() );
      
      KDevelop::TypeAliasType::Ptr alias = type.cast<KDevelop::TypeAliasType>();
      if(alias) {
        //If the aliased type has less involved template arguments, prefer it
        AbstractType::Ptr shortenedTarget = exchange(alias->type());
        if(shortenedTarget && shortenedTarget->toString().count('<') < alias->toString().count('<') && reservedIdentifierCount(shortenedTarget->toString()) <= reservedIdentifierCount(alias->toString())) {
          shortenedTarget->setModifiers(shortenedTarget->modifiers() | alias->modifiers());
          return shortenedTarget;
        }
      }
      
      newType->exchangeTypes(this);
      
      return newType;
    }
  };
  
  ShortenAliasExchanger exchanger;
  type = exchanger.exchange(type);
  return type;
}

///Returns a type that has all template types replaced with DelayedType's that have their template default parameters stripped away,
///and all scope prefixes removed that are redundant within the given context
///The returned type should not actively be used in the  type-system, but rather only for displaying.
AbstractType::Ptr stripType(KDevelop::AbstractType::Ptr type, DUContext* ctx) {
  if(!type)
    return AbstractType::Ptr();
  
  struct ShortenTemplateDefaultParameter : public KDevelop::TypeExchanger {
    DUContext* ctx;
    ShortenTemplateDefaultParameter(DUContext* _ctx) : ctx(_ctx) {
      Q_ASSERT(ctx);
    }
    
    virtual KDevelop::AbstractType::Ptr exchange(const KDevelop::AbstractType::Ptr& type) {
      if(!type)
        return type;
      
      KDevelop::AbstractType::Ptr newType( type->clone() );
      
      if(const KDevelop::IdentifiedType* idType = dynamic_cast<const IdentifiedType*>(type.unsafeData())) {
        KDevelop::Declaration* decl = idType->declaration(ctx->topContext());
        if(!decl)
          return type;
        
        QualifiedIdentifier newTypeName;
        
        if(TemplateDeclaration* tempDecl = dynamic_cast<TemplateDeclaration*>(decl)) 
        {
          if(decl->context()->type() == DUContext::Class && decl->context()->owner()) {
          //Strip template default-parameters from the parent class
            AbstractType::Ptr parentType = stripType(decl->context()->owner()->abstractType(), ctx);
            if(parentType) {
              newTypeName = QualifiedIdentifier(parentType->toString(), true);
            }
          }
          if(newTypeName.isEmpty())
            newTypeName = decl->context()->scopeIdentifier(true);
          
          Identifier currentId;
          if(!idType->qualifiedIdentifier().isEmpty())
            currentId = idType->qualifiedIdentifier().last();
          currentId.clearTemplateIdentifiers();
          
          KDevelop::InstantiationInformation instantiationInfo = tempDecl->instantiatedWith().information();
          KDevelop::InstantiationInformation newInformation(instantiationInfo);
          newInformation.templateParametersList().clear();
          
          for(uint neededParameters = 0; neededParameters < instantiationInfo.templateParametersSize(); ++neededParameters) {
            newInformation.templateParametersList().append(instantiationInfo.templateParameters()[neededParameters]);
            AbstractType::Ptr niceParam = stripType(instantiationInfo.templateParameters()[neededParameters].abstractType(), ctx);
            if(niceParam) {
              currentId.appendTemplateIdentifier(IndexedTypeIdentifier(niceParam->toString(), true));
//               kDebug() << "testing param" << niceParam->toString();
            }
            
            if(tempDecl->instantiate(newInformation, ctx->topContext()) == decl) {
//               kDebug() << "got full instantiation";
              break;
            }
          }
          
          newTypeName.push(currentId);
        }else{
          newTypeName = decl->qualifiedIdentifier();
        }
        //Strip unneded prefixes of the scope
        KDevelop::QualifiedIdentifier candidate = newTypeName;
        while(candidate.count() > 1) {
          candidate = candidate.mid(1);
          QList< KDevelop::Declaration* > decls = ctx->findDeclarations(candidate);
          if(decls.isEmpty())
            continue; // type aliases might be available for nested sub scopes, hence we must not break early
          if(decls[0]->kind() != Declaration::Type || TypeUtils::removeConstModifier(decls[0]->indexedType()) != TypeUtils::removeConstModifier(type->indexed()))
            break;
          newTypeName = candidate;
        }
        if(newTypeName == decl->qualifiedIdentifier())
          return type;
        
        DelayedType::Ptr ret(new KDevelop::DelayedType);
        IndexedTypeIdentifier ti(newTypeName);
        ti.setIsConstant(type->modifiers() & AbstractType::ConstModifier);
        ret->setIdentifier(ti);
        return ret.cast<AbstractType>();
      }      
      newType->exchangeTypes(this);
      
      return newType;
    }
  };
  
  ShortenTemplateDefaultParameter exchanger(ctx);
  type = exchanger.exchange(type);
  return type;
}

AbstractType::Ptr typeForShortenedString (Declaration* decl)
{
  AbstractType::Ptr type = decl->abstractType();
  if(decl->isTypeAlias()) {
      if(type.cast<TypeAliasType>())
        type = type.cast<TypeAliasType>()->type();
  }

  if(decl->isFunctionDeclaration()) {
    FunctionType::Ptr funType = decl->type<FunctionType>();
    if(!funType)
      return AbstractType::Ptr();
    type = funType->returnType();
  }
  return type;
}

QString shortenedTypeString(KDevelop::Declaration* decl, KDevelop::DUContext* ctx, int desiredLength, KDevelop::QualifiedIdentifier stripPrefix) {
  return shortenedTypeString(typeForShortenedString(decl), ctx, desiredLength, stripPrefix);
}

QString simplifiedTypeString(KDevelop::AbstractType::Ptr type, KDevelop::DUContext* visibilityFrom) {
  return shortenedTypeString(type, visibilityFrom, 100000);
}

QString shortenedTypeString(KDevelop::AbstractType::Ptr type, KDevelop::DUContext* ctx, int desiredLength, KDevelop::QualifiedIdentifier stripPrefix)
{
  return shortenedTypeIdentifier(type, ctx, desiredLength, stripPrefix).toString();
}

IndexedTypeIdentifier shortenedTypeIdentifier(AbstractType::Ptr type, DUContext* ctx, int desiredLength, QualifiedIdentifier stripPrefix)
{

  bool isReference = false;
  bool isRValue = false;
  if(ReferenceType::Ptr refType = type.cast<ReferenceType>()) {
    isReference = true;
    type = refType->baseType();
    isRValue = refType->isRValue();
  }

  type = shortenTypeForViewing(type);

  if(ctx)
    type = stripType(type, ctx);
  if(!type)
    return IndexedTypeIdentifier();
  
  IndexedTypeIdentifier identifier = identifierForType(type, ctx ? ctx->topContext() : 0);
  
  if(type.cast<DelayedType>())
    identifier = type.cast<DelayedType>()->identifier();
  identifier = stripPrefixIdentifiers(identifier, stripPrefix);

  if(isReference)
    identifier.setIsReference(true);
  if(isRValue)
    identifier.setIsRValue(true);
  
//   if(identifier.toString().length() > desiredLength)
//     identifier = Cpp::unTypedefType(decl, identifier);
  
  int removeTemplateParametersFrom = 10;
  
  while(identifier.toString().length() > desiredLength * 3 && removeTemplateParametersFrom >= 0) {
    --removeTemplateParametersFrom;
    identifier = removeTemplateParameters(identifier, removeTemplateParametersFrom);
  }
  return identifier;
}

bool isFriend(KDevelop::Declaration* _class, KDevelop::Declaration* _friend) {
  if(!_class || !_friend)
    return false;
  
  DUContext* classInternal = _class->internalContext();
  
  if(!classInternal)
    return false;
  
  static IndexedIdentifier friendIdentifier(Identifier("friend"));
  
  ///@todo Make this more efficient
  QList<Declaration*> decls = classInternal->findLocalDeclarations(friendIdentifier.identifier());
  
  foreach(Declaration* decl, decls)
    if(decl->indexedType() == _friend->indexedType())
      return true;
  
  return false;
}

DUContext* getTemplateContext(DUContext* internal, const TopDUContext* source) {
  if(internal->type() == DUContext::Template)
    return internal;
  
  if(!source)
    source = internal->topContext();
  
  foreach( const DUContext::Import &ctx, internal->importedParentContexts() ) {
    DUContext* c = ctx.context(source);
    if( c ) {
      if( c->type() == DUContext::Template )
        return c;
      c = getTemplateContext(c, source);
      if(c)
        return c;
    }
  }
  return 0;
}

///Returns the context assigned to the given declaration that contains the template-parameters, if available. Else zero.
DUContext* getTemplateContext(Declaration* decl, const TopDUContext* source) {
  DUContext* internal = decl->internalContext();
  if( !internal )
    return 0;
  
  return getTemplateContext(internal, source);
}

QualifiedIdentifier stripPrefixes(DUContext* ctx, QualifiedIdentifier id)
{
  if(!ctx)
    return id;

  QList<QualifiedIdentifier> imports = ctx->fullyApplyAliases(QualifiedIdentifier(), ctx->topContext());
  if(imports.contains(id))
    return QualifiedIdentifier(); ///The id is a namespace that is imported into the current context
  
  QList< Declaration* > basicDecls = ctx->findDeclarations(id, CursorInRevision::invalid(), AbstractType::Ptr(), 0, (DUContext::SearchFlags)(DUContext::NoSelfLookUp | DUContext::NoFiltering));
  
  if(basicDecls.isEmpty())
    return id;
  
  QualifiedIdentifier newId = id.mid(1);
  while(!newId.isEmpty())
  {
    QList< Declaration* > foundDecls = ctx->findDeclarations(newId, CursorInRevision::invalid(), AbstractType::Ptr(), 0, (DUContext::SearchFlags)(DUContext::NoSelfLookUp | DUContext::NoFiltering));

    if(foundDecls == basicDecls)
      id = newId; // must continue to find the shortest possible identifier
                  // esp. for cases where nested namespaces are used (e.g. using namespace a::b::c;)

    newId = newId.mid(1);
  }

  return id;
}

const KDevelop::IndexedIdentifier& castIdentifier()
{
  static const KDevelop::IndexedIdentifier id(Identifier("operator{...cast...}"));
  return id;
}

const KDevelop::IndexedIdentifier& unnamedNamespaceIdentifier()
{
  static const KDevelop::IndexedIdentifier id(Identifier("unnamed{...namespace...}"));
  return id;
}

}

