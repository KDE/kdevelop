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

namespace {

QString typeToString(DUContext::ContextType type)
{
  switch(type) {
    case DUContext::Global: return "Global";
    case DUContext::Namespace: return "Namespace";
    case DUContext::Class: return "Class";
    case DUContext::Function: return "Function";
    case DUContext::Template: return "Template";
    case DUContext::Enum: return "Enum";
    case DUContext::Helper: return "Helper";
    case DUContext::Other: return "Other";
  }
  Q_ASSERT(false);
  return QString();
}

}

class DumpChain
{
public:
  DumpChain();
  ~DumpChain();

  void dumpProblems(DUContext* context);
  void dump( DUContext * context, int allowedDepth );

private:
  int indent;
  QSet<DUContext*> had;
};

DumpChain::DumpChain()
  : indent(0)
{
}

DumpChain::~DumpChain( )
{
}

class Indent
{
public:
  Indent(int level): m_level(level) {}

  friend QDebug& operator<<(QDebug& debug, const Indent& ind) {
    for (int i=0; i<ind.m_level; i++) {
      debug.nospace() << ' ';
    }
    return debug.space();
  }

private:
  int m_level;
};

void DumpChain::dumpProblems(DUContext* context)
{
  QTextStream globalOut(stdout);
  QDebug qout(globalOut.device());

  auto top = context->topContext();
  if (!top->problems().isEmpty()) {
      qout << "Problems:" << endl;
      foreach(const ProblemPointer& p, top->problems()) {
          qout << Indent(indent * 2) << p->description() << p->explanation() << p->finalLocation().textRange() << endl;
      }
      qout << endl;
  }
}

void DumpChain::dump( DUContext * context, int allowedDepth )
{
  QTextStream globalOut(stdout);
  QDebug qout(globalOut.device());

  qout << Indent(indent * 2) << (indent ? "==import==> Context " : "New Context ") << typeToString(context->type()) << context << "\"" <<  context->localScopeIdentifier() << "\" [" << context->scopeIdentifier() << "]"
    << context->range().castToSimpleRange().textRange()
    << (dynamic_cast<TopDUContext*>(context) ? "top-context" : "") << endl;

  auto top = context->topContext();
  if (allowedDepth >= 0) {
    foreach (Declaration* dec, context->localDeclarations(top)) {

      //IdentifiedType* idType = dynamic_cast<IdentifiedType*>(dec->abstractType().data());
      
      qout << Indent((indent+1) * 2) << "Declaration:" << dec->toString() << "[" << dec->qualifiedIdentifier() << "]"
        << dec << "(internal ctx" << dec->internalContext() << ")" << dec->range().castToSimpleRange().textRange() << ","
        << (dec->isDefinition() ? "defined, " : (FunctionDefinition::definition(dec) ? "" : "no definition, "))
        << dec->uses().count() << "use(s)." << endl;
      if (FunctionDefinition::definition(dec)) {
        qout << Indent((indent+1) * 2 + 1) << "Definition:" << FunctionDefinition::definition(dec)->range().castToSimpleRange().textRange() << endl;
      }
      QMap<IndexedString, QList<RangeInRevision> > uses = dec->uses();
      for(QMap<IndexedString, QList<RangeInRevision> >::const_iterator it = uses.constBegin(); it != uses.constEnd(); ++it) {
        qout << Indent((indent+2) * 2) << "File:" << it.key().str() << endl;
        foreach (const RangeInRevision& range, *it)
          qout << Indent((indent+2) * 2+1) << "Use:" << range.castToSimpleRange().textRange() << endl;
      }
    }
  } else {
    qout << Indent((indent+1) * 2) << context->localDeclarations(top).count()
      << "Declarations, " << context->childContexts().size() << "child-contexts" << endl;
  }

  ++indent;
  {
    foreach (const DUContext::Import &parent, context->importedParentContexts()) {
      DUContext* import = parent.context(top);
      if(!import) {
          qout << Indent((indent+2) * 2+1) << "Could not get parent, is it registered in the DUChain?" << endl;
          continue;
      }

      if(had.contains(import)) {
        qout << Indent((indent+2) * 2+1) << "skipping" << import->scopeIdentifier(true) << "because it was already printed" << endl;
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
  dumper.dumpProblems(context);
  dumper.dump(context, allowedDepth);
}
