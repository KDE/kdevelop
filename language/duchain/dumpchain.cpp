/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#include "dumpchain.h"

#include <QtCore/QString>
#include <QTextStream>

#include <kdebug.h>
#include <ktexteditor/range.h>

//#include <identifiedtype.h>
#include <ducontext.h>
#include <topducontext.h>
#include <declaration.h>
#include <duchainpointer.h>
#include <identifier.h>
#include <use.h>

using namespace KDevelop;


DumpChain::DumpChain()
  : indent(0)
{
}

DumpChain::~ DumpChain( )
{
}

void DumpChain::dump( DUContext * context, bool imported )
{
  kDebug() << QString(indent * 2, ' ') << (imported ? "==import==> Context " : "New Context ") << context << "\"" <<  context->localScopeIdentifier() << "\" [" << context->scopeIdentifier() << "]" << context->range().textRange() << " " << (dynamic_cast<TopDUContext*>(context) ? "top-context" : "");
  if( !context )
    return;
  if (!imported) {
    foreach (Declaration* dec, context->localDeclarations()) {
      
      //IdentifiedType* idType = dynamic_cast<IdentifiedType*>(dec->abstractType().data());
      
      kDebug() << QString((indent+1) * 2, ' ') << "Declaration: " << dec->toString() << /*(idType ? (" (type-identity: " + idType->identifier().toString() + ")") : QString()) <<*/ " [" << dec->qualifiedIdentifier() << "]" << dec << "(internal ctx" << dec->internalContext() << ")" << dec->range().textRange() << "," << (dec->isDefinition() ? "defined, " : (dec->definition() ? "" : "no definition, ")) << dec->uses().count() << "use(s).";
      if (dec->definition())
        kDebug() << QString((indent+1) * 2 + 1, ' ') << "Definition:" << dec->definition()->range().textRange();
      QMap<HashedString, QList<SimpleRange> > uses = dec->uses();
      for(QMap<HashedString, QList<SimpleRange> >::const_iterator it = uses.begin(); it != uses.end(); ++it) {
        kDebug() << QString((indent+2) * 2, ' ') << "File:" << it.key().str();
        foreach (const SimpleRange& range, *it)
          kDebug() << QString((indent+2) * 2+1, ' ') << "Use:" << range.textRange();
      }
    }
  }

  ++indent;
  if (!imported) {
    ///@todo Think whether this is used for top-contexts, and if it is, prevent endless recursion due to loops
    foreach (DUContextPointer parent, context->importedParentContexts()) {
      dump(parent.data(), true);
    }

    foreach (DUContext* child, context->childContexts())
      dump(child);
  }
  --indent;
}


