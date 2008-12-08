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
  for( QVector<DUContext::Import>::const_iterator it = bases.begin(); it != bases.end(); ++it ) {
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

Declaration* localClassFromCodeContext(DUContext* context)
{
  if(!context)
    return 0;
  
  DUContext* startContext = context;
  
  while( context->parentContext() && context->type() == DUContext::Other && context->parentContext()->type() == DUContext::Other )
  { //Move context to the top context of type "Other". This is needed because every compound-statement creates a new sub-context.
    context = context->parentContext();
  }
  
  //Since declarations are assigned to the bodies, not to the argument contexts, go to the body context to make the step to the function
  if(context->type() == DUContext::Function)
    if(!context->importers().isEmpty())
      context = context->importers().first();

  if(!context) {
    kDebug() << "problem";
    return 0;
  }
  
  ///Step 1: Find the function-declaration for the function we are in
  Declaration* functionDeclaration = 0;

  if( context->owner() && dynamic_cast<FunctionDefinition*>(context->owner()) )
    functionDeclaration = static_cast<FunctionDefinition*>(context->owner())->declaration(startContext->topContext());
  else{
    ///Alternative way of finding the class, needed while building the duchain when the links are incomplete
    QVector<DUContext::Import> imports = context->importedParentContexts();
    foreach(const DUContext::Import& import, imports) {
      DUContext* imp = import.context(context->topContext());
      if(imp && imp->type() == DUContext::Class && imp->owner())
        return imp->owner();
    }
  }

  if( !functionDeclaration && context->owner() )
    functionDeclaration = context->owner();

  if(!functionDeclaration)
    return 0;

  return functionDeclaration->context()->owner();
}

KDEVCPPDUCHAIN_EXPORT bool isAccessible(DUContext* /*fromContext*/, Declaration* /*declaration*/)
{
  ///@todo implement
  return true;
}

/**
 * Preprocess the given string using the macros from given EnvironmentFile up to the given line
 * If line is -1, all macros are respected.
 * This is a quite slow operation, because thousands of macros need to be shuffled around.
 * 
 * @todo maybe implement a version of rpp::Environment that directly works on EnvironmentFile,
 * without needing to copy all macros.
 * */
QString preprocess( const QString& text, Cpp::EnvironmentFile* file, int line ) {

  rpp::Preprocessor preprocessor;
  rpp::pp pp(&preprocessor);

  {
      DUChainReadLocker lock(DUChain::lock());
/*    kDebug(9007) << "defined macros: " << file->definedMacros().size();*/
    //Copy in all macros from the file
    for( Cpp::ReferenceCountedMacroSet::Iterator it( file->definedMacros().iterator() ); it; ++it ) {
      if( line == -1 || line > it.ref().sourceLine || file->url() != it.ref().file ) {
        pp.environment()->setMacro( copyConstantMacro( &it.ref() ) );
/*        kDebug(9007) << "adding macro " << (*it).name.str();*/
      } else {
/*        kDebug(9007) << "leaving macro " << (*it).name.str();*/
      }
    }
/*    kDebug(9007) << "used macros: " << file->usedMacros().size();*/
    for( Cpp::ReferenceCountedMacroSet::Iterator it( file->usedMacros().iterator() ); it; ++it ) {
      if( line == -1 || line > it.ref().sourceLine || file->url() != it.ref().file ) {
        pp.environment()->setMacro( copyConstantMacro(&it.ref()) );
/*        kDebug(9007) << "adding macro " << (*it).name.str();*/
      } else {
/*        kDebug(9007) << "leaving macro " << (*it).name.str();*/
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
  for(int a = 0; a < id.templateIdentifiersCount(); ++a)
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
  for(int a = 0; a < id.templateIdentifiersCount(); ++a) {
    TypeIdentifier replacement = removeTemplateParameters(id.templateIdentifier(a), behindPosition);
    if(a < behindPosition)
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

QString shortenedTypeString(Declaration* decl, int desiredLength) {
  AbstractType::Ptr type = decl->abstractType();
  if(decl->isFunctionDeclaration()) {
    FunctionType::Ptr funType = decl->type<FunctionType>();
    if(!funType)
      return QString();
    type = funType->returnType();
  }
  
  if(!type)
    return QString();

  TypeIdentifier identifier = TypeIdentifier(type->toString());
  
  if(identifier.toString().length() > desiredLength)
    identifier = Cpp::unTypedefType(decl, identifier);
  
  bool doneSomething = true;
  
  int removeTemplateParametersFrom = 10;
  
  ///@todo Remove namespace-prefixes
  
  while(identifier.toString().length() > desiredLength * 3 && removeTemplateParametersFrom >= 0) {
    --removeTemplateParametersFrom;
    identifier = removeTemplateParameters(identifier, removeTemplateParametersFrom);
  }
  
  return identifier.toString();
}
}

