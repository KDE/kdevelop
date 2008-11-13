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
#include <ktexteditor/smartrange.h>

#include "ducontext.h"
#include "topducontext.h"
#include "declaration.h"
#include "duchainpointer.h"
#include "identifier.h"
#include "use.h"
#include "indexedstring.h"
#include "functiondefinition.h"

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
          enum ContextType {
    Global    /**< A context that declares functions, namespaces or classes */,
    Namespace /**< A context that declares namespace members */,
    Class     /**< A context that declares class members */,
    Function  /**< A context that declares function-arguments */,
    Template  /**< A context that declares template-parameters */,
    Enum      /**< A context that contains a list of enumerators */,
    Helper    /**< A helper context, used for language-specific tweaks */,
    Other     /**< Represents executable code, like for example within a compound-statement */
  };
  QString type;
  switch(context->type()) {
    case Global: type = "Global"; break;
    case Namespace: type = "Namespace"; break;
    case Class: type = "Class"; break;
    case Function: type = "Function"; break;
    case Template: type = "Template"; break;
    case Enum: type = "Enum"; break;
    case Helper: type = "Helper"; break;
    case Other: type = "Other"; break;
  }
  kDebug() << QString(indent * 2, ' ') << (imported ? "==import==> Context " : "New Context ") << type << context << "\"" <<  context->localScopeIdentifier() << "\" [" << context->scopeIdentifier() << "]" << context->range().textRange() << " " << (dynamic_cast<TopDUContext*>(context) ? "top-context" : "");

      
  if( !context )
    return;
  if (!imported) {
    foreach (Declaration* dec, context->localDeclarations()) {
      
      //IdentifiedType* idType = dynamic_cast<IdentifiedType*>(dec->abstractType().data());
      
      kDebug() << QString((indent+1) * 2, ' ') << "Declaration: " << dec->toString() << /*(idType ? (" (type-identity: " + idType->identifier().toString() + ")") : QString()) <<*/ " [" << dec->qualifiedIdentifier() << "]" << dec << "(internal ctx" << dec->internalContext() << ")" << dec->range().textRange() << "smart range:" << dec->smartRange() << "," << (dec->isDefinition() ? "defined, " : (FunctionDefinition::definition(dec) ? "" : "no definition, ")) << dec->uses().count() << "use(s).";
      if (FunctionDefinition::definition(dec))
        kDebug() << QString((indent+1) * 2 + 1, ' ') << "Definition:" << FunctionDefinition::definition(dec)->range().textRange();
      QMap<IndexedString, QList<SimpleRange> > uses = dec->uses();
      for(QMap<IndexedString, QList<SimpleRange> >::const_iterator it = uses.begin(); it != uses.end(); ++it) {
        kDebug() << QString((indent+2) * 2, ' ') << "File:" << it.key().str();
        foreach (const SimpleRange& range, *it)
          kDebug() << QString((indent+2) * 2+1, ' ') << "Use:" << range.textRange();
      }
    }
  }

  ++indent;
  if (!imported) {
    ///@todo Think whether this is used for top-contexts, and if it is, prevent endless recursion due to loops
    foreach (DUContext::Import parent, context->importedParentContexts()) {
      if(!parent.context(context->topContext())) {
          kDebug() << "Could not get parent, is it registered in the DUChain?";
          continue;
      }
      dump(parent.context(context->topContext()), true);
    }

    foreach (DUContext* child, context->childContexts())
      dump(child);
  }
  --indent;
}

QString DumpChain::dumpRanges(KTextEditor::SmartRange* range, QString indent)
{
  QString ret;
  QTextStream stream(&ret);
  stream << indent << range << "(" << range->start().line() << ", " <<  range->start().column() << ") -> (" << range->end().line() << ", " <<  range->end().column() << ")" << "\n";
  foreach(KTextEditor::SmartRange* child, range->childRanges())
    stream << dumpRanges(child, indent + " ");
  
  return ret;
}
