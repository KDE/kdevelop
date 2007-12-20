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
#include <duchainpointer.h>
#include "cppduchainexport.h"


using namespace Cpp;
using namespace KDevelop;

namespace Cpp {

KDEVCPPDUCHAIN_EXPORT  QList<KDevelop::Declaration*> findLocalDeclarations( KDevelop::DUContext* context, const KDevelop::QualifiedIdentifier& identifier ) {
  QList<Declaration*> ret;
  ret += context->findLocalDeclarations( identifier );
  if( !ret.isEmpty() )
    return ret;

  if( context->type() != DUContext::Class )
    return ret;
  
  QList<DUContextPointer> bases = context->importedParentContexts();
  for( QList<DUContextPointer>::const_iterator it = bases.begin(); it != bases.end(); ++it ) {
    if( *it )
      ret += findLocalDeclarations( (*it).data(), identifier );
  }
  return ret;
}

KDEVCPPDUCHAIN_EXPORT  QList<KDevelop::Declaration*> localDeclarations( KDevelop::DUContext* context ) {
  QList<Declaration*> ret;
  ret += context->localDeclarations();
  if( !ret.isEmpty() )
    return ret;

  if( context->type() != DUContext::Class )
    return ret;

  ///@todo exclude overloaded functions
  QList<DUContextPointer> bases = context->importedParentContexts();
  for( QList<DUContextPointer>::const_iterator it = bases.begin(); it != bases.end(); ++it ) {
    if( *it )
      ret += localDeclarations( (*it).data() );
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

}

