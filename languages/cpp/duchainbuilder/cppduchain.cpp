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
#include <duchainpointer.h>
#include "cppduchainbuilderexport.h"


using namespace Cpp;
using namespace KDevelop;

namespace Cpp {

KDEVCPPDUCHAINBUILDER_EXPORT  QList<KDevelop::Declaration*> findLocalDeclarations( KDevelop::DUContext* context, const KDevelop::QualifiedIdentifier& identifier ) {
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

KDEVCPPDUCHAINBUILDER_EXPORT  QList<KDevelop::Declaration*> localDeclarations( KDevelop::DUContext* context ) {
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

}
