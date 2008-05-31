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
#include <ducontext.h>
#include <identifier.h>
#include <declaration.h>
#include <duchainlock.h>
#include <duchain.h>
#include <duchainpointer.h>
#include "environmentmanager.h"
#include "parser/rpp/pp-engine.h"
#include "parser/rpp/preprocessor.h"
#include "parser/rpp/pp-environment.h"
#include "parser/rpp/pp-macro.h"
#include "cppduchainexport.h"


using namespace Cpp;
using namespace KDevelop;

namespace Cpp {

KDEVCPPDUCHAIN_EXPORT  QList<KDevelop::Declaration*> findLocalDeclarations( KDevelop::DUContext* context, const KDevelop::QualifiedIdentifier& identifier, const TopDUContext* topContext ) {
  QList<Declaration*> ret;
  ret += context->findLocalDeclarations( identifier, SimpleCursor::invalid(), topContext );
  if( !ret.isEmpty() )
    return ret;

  if( context->type() != DUContext::Class )
    return ret;
  
  QVector<DUContextPointer> bases = context->importedParentContexts();
  for( QVector<DUContextPointer>::const_iterator it = bases.begin(); it != bases.end(); ++it ) {
    if( *it )
      ret += findLocalDeclarations( (*it).data(), identifier, topContext );
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
  }

  QList< QPair<Declaration*, int> > ret;
    
  ///Only keep the declarations of each name on the lowest inheritance-level
  foreach( const Pair& decl, declarations )
    if( nearestDeclaration[decl.first->identifier()] == decl.first )
      ret << decl;

  return ret;
}

QList<KDevelop::Declaration*> findDeclarationsSameLevel(KDevelop::DUContext* context, const QualifiedIdentifier& identifier, const KDevelop::SimpleCursor& position)
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
    if(!context->importedChildContexts().isEmpty())
      context = context->importedChildContexts().first();

  ///Step 1: Find the function-declaration for the function we are in
  Declaration* functionDeclaration = 0;

  if( context->owner() && context->owner()->isDefinition() )
    functionDeclaration = context->owner()->declaration(startContext->topContext());

  if( !functionDeclaration && context->owner() )
    functionDeclaration = context->owner();

  if(!functionDeclaration)
    return 0;

  return functionDeclaration->context()->owner();
}

KDEVCPPDUCHAIN_EXPORT bool isAccessible(DUContext* fromContext, Declaration* declaration)
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
    for( Cpp::MacroRepository::Iterator it( &Cpp::EnvironmentManager::m_macroRepository, file->definedMacros().set().iterator() ); it; ++it ) {
      if( line == -1 || line > (*it).sourceLine || file->url() != (*it).file ) {
        pp.environment()->setMacro( new rpp::pp_macro( *it ) );
/*        kDebug(9007) << "adding macro " << (*it).name.str();*/
      } else {
/*        kDebug(9007) << "leaving macro " << (*it).name.str();*/
      }
    }
/*    kDebug(9007) << "used macros: " << file->usedMacros().size();*/
    for( Cpp::MacroRepository::Iterator it( &Cpp::EnvironmentManager::m_macroRepository, file->usedMacros().set().iterator() ); it; ++it ) {
      if( line == -1 || line > (*it).sourceLine || file->url() != (*it).file ) {
        pp.environment()->setMacro( new rpp::pp_macro( *it ) );
/*        kDebug(9007) << "adding macro " << (*it).name.str();*/
      } else {
/*        kDebug(9007) << "leaving macro " << (*it).name.str();*/
      }
    }
  }

  QString ret = pp.processFile("anonymous", rpp::pp::Data, text.toUtf8());
  pp.environment()->cleanup();
  
  return ret;
}
}

