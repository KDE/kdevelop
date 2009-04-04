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

namespace Cpp {

KDEVCPPDUCHAIN_EXPORT  QList<KDevelop::Declaration*> findLocalDeclarations( KDevelop::DUContext* context, const KDevelop::Identifier& identifier, const TopDUContext* topContext ) {
  QList<Declaration*> ret;
  ret += context->findLocalDeclarations( identifier, SimpleCursor::invalid(), topContext );
  if( !ret.isEmpty() )
    return ret;

  if( context->type() != DUContext::Class )
    return ret;
  
  QVector<DUContext::Import> bases = context->importedParentContexts();
  for( QVector<DUContext::Import>::const_iterator it = bases.constBegin(); it != bases.constEnd(); ++it ) {
    if( it->context(topContext) )
      ret += findLocalDeclarations( (*it).context(topContext), identifier, topContext );
  }
  return ret;
}

void minimize(int& i, const int with) {
  if( with < i )
    i = with;
}

QList< QPair<Declaration*, int> > hideOverloadedDeclarations( const QList< QPair<Declaration*, int> >& declarations ) {
  QHash<Identifier, Declaration*> nearestDeclaration;
  QHash<Declaration*, int> depthHash;
  QSet<Identifier> hadNonForwardDeclaration; //Set of all non function-declarations that had a declaration that was not a forward-declaration.

  typedef QPair<Declaration*, int> Pair;
  foreach(  const Pair& decl, declarations ) {
    depthHash[decl.first] = decl.second;
    
    QHash<Identifier, Declaration*>::iterator it = nearestDeclaration.find(decl.first->identifier());

    if(it == nearestDeclaration.end()) {
      nearestDeclaration[ decl.first->identifier() ] = decl.first;
    }else if(decl.first->isForwardDeclaration() == (*it)->isForwardDeclaration() || (!decl.first->isForwardDeclaration() && (*it)->isForwardDeclaration())){
      //Always prefer non forward-declarations over forward-declarations
      if((!decl.first->isForwardDeclaration() && (*it)->isForwardDeclaration()) || decl.second < depthHash[*it])
        nearestDeclaration[ decl.first->identifier() ] = decl.first;
    }
    
    if(!decl.first->isForwardDeclaration() && !decl.first->isFunctionDeclaration())
      hadNonForwardDeclaration.insert(decl.first->identifier());
  }

  QList< QPair<Declaration*, int> > ret;
    
  ///Only keep the declarations of each name on the lowest inheritance-level, or that are not forward-declarations
  foreach( const Pair& decl, declarations ) {
    if( depthHash[nearestDeclaration[decl.first->identifier()]] == decl.second ) {
      if(decl.first->isFunctionDeclaration() || !decl.first->isForwardDeclaration() || !hadNonForwardDeclaration.contains(decl.first->identifier()))
        ret << decl;
    }
  }

  return ret;
}

QList<KDevelop::Declaration*> findDeclarationsSameLevel(KDevelop::DUContext* context, const Identifier& identifier, const KDevelop::SimpleCursor& position)
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
    prefix.pop();
  }
  
  if(classContext) {
    while(!prefix.isEmpty() && classContext && classContext->type() == DUContext::Class) {
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

Declaration* localClassFromCodeContext(DUContext* context)
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
    foreach(DUContext::Import import, context->importedParentContexts()) {
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
    foreach(DUContext::Import import, context->importedParentContexts()) {
      DUContext* ctx = import.context(context->topContext());
      if(ctx && ctx->type() == DUContext::Class && ctx->owner())
        return ctx->owner();
    }
    
    if(!context->importers().isEmpty())
      context = context->importers().first();
  }

  return 0;
}

Declaration* localFunctionFromCodeContext(DUContext* context)
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
    foreach(DUContext::Import import, context->importedParentContexts()) {
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
 * 
 * @todo maybe implement a version of rpp::Environment that directly works on EnvironmentFile,
 * without needing to copy all macros.
 * */
QString preprocess( const QString& text, Cpp::EnvironmentFile* file, int line, QSet<IndexedString> disableMacros ) {

  rpp::Preprocessor preprocessor;
  rpp::pp pp(&preprocessor);

  {
      DUChainReadLocker lock(DUChain::lock());
/*    kDebug(9007) << "defined macros: " << file->definedMacros().size();*/
    //Copy in all macros from the file
    for( Cpp::ReferenceCountedMacroSet::Iterator it( file->definedMacros().iterator() ); it; ++it ) {
      if( line == -1 || line > it.ref().sourceLine || file->url() != it.ref().file ) {
        if(!disableMacros.contains( it.ref().name ))
          pp.environment()->setMacro( copyConstantMacro( &it.ref() ) );
      }
    }
    for( Cpp::ReferenceCountedMacroSet::Iterator it( file->usedMacros().iterator() ); it; ++it ) {
      if( line == -1 || line > it.ref().sourceLine || file->url() != it.ref().file ) {
        if(!disableMacros.contains( it.ref().name ))
          pp.environment()->setMacro( copyConstantMacro(&it.ref()) );
      }
    }
  }

  QString ret = QString::fromUtf8(stringFromContents(pp.processFile("anonymous", text.toUtf8())));
  pp.environment()->cleanup();
  
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

KDevelop::TypeIdentifier exchangeQualifiedIdentifier(KDevelop::TypeIdentifier id, KDevelop::QualifiedIdentifier replace, KDevelop::QualifiedIdentifier replaceWith) {
  KDevelop::TypeIdentifier ret(id);
  while(ret.count())
    ret.pop();
  if(QualifiedIdentifier(id) == replace) {
    for(int a = 0; a < replaceWith.count(); ++a)
      ret.push(replaceWith.at(a));
  }else{
    for(int a = 0; a < id.count(); ++a)
      ret.push(exchangeQualifiedIdentifier(id.at(a), replace, replaceWith));
  }
  return ret;
}

KDevelop::TypeIdentifier unTypedefType(Declaration* decl, KDevelop::TypeIdentifier type) {
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

TypeIdentifier removeTemplateParameters(TypeIdentifier identifier, int behindPosition);

Identifier removeTemplateParameters(Identifier id, int behindPosition) {
  Identifier ret(id);
  ret.clearTemplateIdentifiers();
  for(unsigned int a = 0; a < id.templateIdentifiersCount(); ++a) {
    TypeIdentifier replacement = removeTemplateParameters(id.templateIdentifier(a), behindPosition);
    if((int) a < behindPosition)
      ret.appendTemplateIdentifier(replacement);
    else {
      ret.appendTemplateIdentifier(TypeIdentifier("..."));
      break;
    }
  }
  return ret;
}

TypeIdentifier removeTemplateParameters(TypeIdentifier identifier, int behindPosition) {
  TypeIdentifier ret(identifier);
  while(ret.count())
    ret.pop();
  
  for(int a = 0; a < identifier.count(); ++a)
    ret.push(removeTemplateParameters(identifier.at(a), behindPosition));

  return ret;
}

KDevelop::TypeIdentifier stripPrefixIdentifiers(KDevelop::TypeIdentifier id, KDevelop::QualifiedIdentifier strip);

KDevelop::Identifier stripPrefixIdentifiers(KDevelop::Identifier id, KDevelop::QualifiedIdentifier strip) {
  KDevelop::Identifier ret(id);
  ret.clearTemplateIdentifiers();
  for(unsigned int a = 0; a < id.templateIdentifiersCount(); ++a)
    ret.appendTemplateIdentifier(stripPrefixIdentifiers(id.templateIdentifier(a), strip));

  return ret;
}

KDevelop::TypeIdentifier stripPrefixIdentifiers(KDevelop::TypeIdentifier id, KDevelop::QualifiedIdentifier strip) {
  
  KDevelop::TypeIdentifier ret(id);
  while(ret.count())
    ret.pop();
  
  int commonPrefix = 0;
  for(;commonPrefix < id.count()-1 && commonPrefix < strip.count(); ++commonPrefix)
    if(strip.at(commonPrefix).toString() != id.at(commonPrefix).toString())
      break;
  
  for(int a = commonPrefix; a < id.count(); ++a)
    ret.push( stripPrefixIdentifiers(id.at(a), strip) );

  return ret;
}

int reservedIdentifierCount(QString name) {
  QStringList l = name.split("::");
  int ret = 0;
  foreach(QString s, l)
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

///Returns a type that has all template types replaced with DelayedType's that have their template default parameters stripped away.
///The returned type should not actively be used in the  type-system, but rather only for displaying.
AbstractType::Ptr stripTemplateDefaultParameters(KDevelop::AbstractType::Ptr type, TopDUContext* top) {
  
  if(!type)
    return AbstractType::Ptr();
  
  struct ShortenTemplateDefaultParameter : public KDevelop::TypeExchanger {
    TopDUContext* top;
    ShortenTemplateDefaultParameter(TopDUContext* _top) : top(_top) {
    }
    
    virtual KDevelop::AbstractType::Ptr exchange(const KDevelop::AbstractType::Ptr& type) {
      if(!type)
        return type;
//       kDebug() << "simplifying" << type->toString();
      
      KDevelop::AbstractType::Ptr newType( type->clone() );
      
      if(const KDevelop::IdentifiedType* idType = dynamic_cast<const IdentifiedType*>(type.unsafeData())) {
        KDevelop::Declaration* decl = idType->declaration(top);
        if(TemplateDeclaration* tempDecl = dynamic_cast<TemplateDeclaration*>(decl)) 
        {
          TypeIdentifier newTypeName;
          if(decl->context()->type() == DUContext::Class && decl->context()->owner()) {
            //Strip template default-parameters from the parent class
            AbstractType::Ptr parentType = stripTemplateDefaultParameters(decl->context()->owner()->abstractType(), top);
            if(parentType)
              newTypeName = TypeIdentifier(parentType->toString());
          }
          if(newTypeName.isEmpty())
            newTypeName = decl->context()->scopeIdentifier(true);
          
          Identifier currentId;
          if(!idType->qualifiedIdentifier().isEmpty())
            currentId.setIdentifier(idType->qualifiedIdentifier().last().identifier().str());
          
          KDevelop::InstantiationInformation instantiationInfo = tempDecl->instantiatedWith().information();
          int neededParameters = 0;
          KDevelop::InstantiationInformation newInformation(instantiationInfo);
          newInformation.templateParametersList().clear();
          
          for(neededParameters = 0; neededParameters < instantiationInfo.templateParametersSize(); ++neededParameters) {
            newInformation.templateParametersList().append(instantiationInfo.templateParameters()[neededParameters]);
            AbstractType::Ptr niceParam = stripTemplateDefaultParameters(instantiationInfo.templateParameters()[neededParameters].abstractType(), top);
            if(niceParam) {
              currentId.appendTemplateIdentifier(niceParam->toString());
//               kDebug() << "testing param" << niceParam->toString();
            }
            
            if(tempDecl->instantiate(newInformation, top) == decl) {
//               kDebug() << "got full instantiation";
              break;
            }
          }
          
          newTypeName.push(currentId);
          
          DelayedType::Ptr ret(new KDevelop::DelayedType);
          ret->setIdentifier(newTypeName);
//           kDebug() << "created delayed type" << ret->toString();
          return ret.cast<AbstractType>();
        }else{
          return type;
        }
      }      
      newType->exchangeTypes(this);
      
      return newType;
    }
  };
  
  ShortenTemplateDefaultParameter exchanger(top);
  type = exchanger.exchange(type);
  return type;
}

QString shortenedTypeString(KDevelop::Declaration* decl, KDevelop::TopDUContext* top, int desiredLength, KDevelop::QualifiedIdentifier stripPrefix) {
  AbstractType::Ptr type = decl->abstractType();
  if(decl->isTypeAlias()) {
      if(type.cast<TypeAliasType>())
        type = type.cast<TypeAliasType>()->type();
  }
  
  if(decl->isFunctionDeclaration()) {
    FunctionType::Ptr funType = decl->type<FunctionType>();
    if(!funType)
      return QString();
    type = funType->returnType();
  }
  
  return shortenedTypeString(type, top, desiredLength, stripPrefix);
}

QString simplifiedTypeString(KDevelop::AbstractType::Ptr type, KDevelop::DUContext* visibilityFrom) {
  ///@todo Nicely respect the source visibility context
  return shortenedTypeString(type, visibilityFrom ? visibilityFrom->topContext() : 0, 100000, visibilityFrom ? visibilityFrom->scopeIdentifier(true) : QualifiedIdentifier());
}

QString shortenedTypeString(KDevelop::AbstractType::Ptr type, TopDUContext* top, int desiredLength, KDevelop::QualifiedIdentifier stripPrefix) {

  bool isReference = false;
  if(type.cast<ReferenceType>()) {
    isReference = true;
    type = type.cast<ReferenceType>()->baseType();
  }

  type = shortenTypeForViewing(type);
  
  if(top)
    type = stripTemplateDefaultParameters(type, top);
  
  if(!type)
    return QString();
  
  TypeIdentifier identifier = TypeIdentifier(type->toString());
  
  if(type.cast<DelayedType>())
    identifier = type.cast<DelayedType>()->identifier();
  
  identifier = stripPrefixIdentifiers(identifier, stripPrefix);

  if(isReference)
    identifier.setIsReference(true);
  
//   if(identifier.toString().length() > desiredLength)
//     identifier = Cpp::unTypedefType(decl, identifier);
  
  int removeTemplateParametersFrom = 10;
  
  ///@todo Remove namespace-prefixes
  
  while(identifier.toString().length() > desiredLength * 3 && removeTemplateParametersFrom >= 0) {
    --removeTemplateParametersFrom;
    identifier = removeTemplateParameters(identifier, removeTemplateParametersFrom);
  }
  
  return identifier.toString();
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

}

