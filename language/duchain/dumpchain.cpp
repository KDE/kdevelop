/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Hamish Rodda <rodda@kde.org>
    Copyright 2010 Milian Wolff <mail@milianw.de>

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

#include <KDebug>

#include "ducontext.h"
#include "topducontext.h"
#include "declaration.h"
#include "duchainpointer.h"
#include "identifier.h"
#include "use.h"
#include "problem.h"
#include "indexedstring.h"
#include "functiondefinition.h"

#include <editor/rangeinrevision.h>

using namespace KDevelop;

//BEGIN: private

class DumpChain
{
public:
  DumpChain();
  ~DumpChain();

  void dump( DUContext * context, int allowedDepth );

private:
  int indent;
  TopDUContext* top;
  QSet<DUContext*> had;
};

DumpChain::DumpChain()
  : indent(0), top(0)
{
}

DumpChain::~DumpChain( )
{
}

class Indent {
private:
  int m_level;
public:
  Indent(int level): m_level(level) {}
  friend QDebug& operator<<(QDebug& debug, const Indent& ind) {
    for (int i=0; i<ind.m_level; i++) {
      debug << ' ';
    }
    return debug;
  }
};

void DumpChain::dump( DUContext * context, int allowedDepth )
{
  QTextStream globalOut(stdout);
  // use a QDebug to utilize operator<<() overloads
  // but don't use kDebug() to make sure we always print it, no matter what
  // is set in kdebugdialog
  QDebug qout(globalOut.device());

  if(!top || top != context->topContext()) {
    top = context->topContext();
    if (!top->problems().isEmpty()) {
        qout << "PROBLEMS:" << endl;
        foreach(const ProblemPointer& p, top->problems()) {
            qout << p->description() << p->explanation() << p->finalLocation().textRange() << endl;
        }
    }
  }

  QString type;
  switch(context->type()) {
    case DUContext::Global: type = "Global"; break;
    case DUContext::Namespace: type = "Namespace"; break;
    case DUContext::Class: type = "Class"; break;
    case DUContext::Function: type = "Function"; break;
    case DUContext::Template: type = "Template"; break;
    case DUContext::Enum: type = "Enum"; break;
    case DUContext::Helper: type = "Helper"; break;
    case DUContext::Other: type = "Other"; break;
  }
  qout << QString(indent * 2, ' ') << (indent ? "==import==> Context " : "New Context ") << type << context << "\"" <<  context->localScopeIdentifier() << "\" [" << context->scopeIdentifier() << "]" << context->range().castToSimpleRange().textRange() << ' ' << (dynamic_cast<TopDUContext*>(context) ? "top-context" : "") << endl;


  if( !context )
    return;
  if (allowedDepth >= 0) {
    foreach (Declaration* dec, context->localDeclarations(top)) {

      //IdentifiedType* idType = dynamic_cast<IdentifiedType*>(dec->abstractType().data());
      
      qout << QString((indent+1) * 2, ' ') << "Declaration: " << dec->toString() << /*(idType ? (" (type-identity: " + idType->identifier().toString() + ")") : QString()) <<*/ " [" << dec->qualifiedIdentifier() << "]" << dec << "(internal ctx" << dec->internalContext() << ")" << dec->range().castToSimpleRange().textRange() << "," << (dec->isDefinition() ? "defined, " : (FunctionDefinition::definition(dec) ? "" : "no definition, ")) << dec->uses().count() << "use(s)." << endl;
      if (FunctionDefinition::definition(dec)) {
        qout << QString((indent+1) * 2 + 1, ' ') << "Definition:" << FunctionDefinition::definition(dec)->range().castToSimpleRange().textRange() << endl;
      }
      QMap<IndexedString, QList<RangeInRevision> > uses = dec->uses();
      for(QMap<IndexedString, QList<RangeInRevision> >::const_iterator it = uses.constBegin(); it != uses.constEnd(); ++it) {
        qout << QString((indent+2) * 2, ' ') << "File:" << it.key().str() << endl;
        foreach (const RangeInRevision& range, *it)
          qout << QString((indent+2) * 2+1, ' ') << "Use:" << range.castToSimpleRange().textRange() << endl;
      }
    }
  } else {
    qout << QString((indent+1) * 2, ' ') << context->localDeclarations(top).count() << "Declarations, " << context->childContexts().size() << "child-contexts" << endl;
  }

  ++indent;
  {
    foreach (const DUContext::Import &parent, context->importedParentContexts()) {
      DUContext* import = parent.context(top);
      if(!import) {
          qout << QString((indent+2) * 2+1, ' ') << "Could not get parent, is it registered in the DUChain?" << endl;
          continue;
      }

      if(had.contains(import)) {
        qout << QString((indent+2) * 2+1, ' ') << "skipping" << import->scopeIdentifier(true) << "because it was already printed" << endl;
        continue;
      }
      had.insert(import);

      dump(import, allowedDepth-1);
    }

    foreach (DUContext* child, context->childContexts())
      dump(child, allowedDepth-1);
  }
  --indent;

  if(indent == 0) {
    top = 0;
    had.clear();
  }
}

//END: private

//BEGIN: public

void KDevelop::dumpDUContext(DUContext* context, int allowedDepth)
{
  DumpChain dumper;
  dumper.dump(context, allowedDepth);
}
