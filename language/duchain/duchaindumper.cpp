/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Hamish Rodda <rodda@kde.org>
    Copyright 2010 Milian Wolff <mail@milianw.de>
    Copyright 2014 Kevin Funk <kfunk@kde.org>

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

#include "duchaindumper.h"

#include <QtCore/QString>
#include <QTextStream>

#include "definitions.h"
#include "ducontext.h"
#include "topducontext.h"
#include "declaration.h"
#include "duchainpointer.h"
#include "identifier.h"
#include "use.h"
#include "problem.h"
#include <serialization/indexedstring.h>
#include "functiondefinition.h"

#include <editor/rangeinrevision.h>

using namespace KDevelop;

namespace {

QDebug fromTextStream(const QTextStream& out) { if (out.device()) return {out.device()}; return {out.string()}; }

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

struct DUChainDumper::Private
{
  Private()
    : m_indent(0)
  {}

  void dumpProblems(TopDUContext* top, QTextStream& out);
  void dump(DUContext* context, int allowedDepth, bool isFromImport, QTextStream& out);

  int m_indent;
  Features m_features;
  QSet<DUContext*> m_visitedContexts;
};

DUChainDumper::DUChainDumper(Features features)
  : d(new Private)
{
  d->m_features = features;
}

DUChainDumper::~DUChainDumper( )
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

void DUChainDumper::Private::dumpProblems(TopDUContext* top, QTextStream& out)
{
  QDebug qout = fromTextStream(out);

  if (!top->problems().isEmpty()) {
      qout << top->problems().size() << "problems encountered:" << endl;
      foreach(const ProblemPointer& p, top->problems()) {
          qout << Indent(m_indent * 2) << p->description() << p->explanation() << p->finalLocation() << endl;
      }
      qout << endl;
  }
}

void DUChainDumper::Private::dump(DUContext * context, int allowedDepth, bool isFromImport, QTextStream& out)
{
  QDebug qout = fromTextStream(out);

  qout << Indent(m_indent * 2) << (isFromImport ? " ==import==>" : "")
    << (dynamic_cast<TopDUContext*>(context) ? "Top-Context" : "Context") << typeToString(context->type())
    << "(owner: " << context->owner() << ")"
    << context << context->localScopeIdentifier() << "[" << context->scopeIdentifier(true) << "]"
    << context->range().castToSimpleRange()
    << (dynamic_cast<TopDUContext*>(context) ? static_cast<TopDUContext*>(context)->url().byteArray(): "")
    << endl;

  if(m_visitedContexts.contains(context)) {
    qout << Indent((m_indent+2) * 2) << "(Skipping" << context->scopeIdentifier(true) << "because it was already printed)" << endl;
    return;
  }

  m_visitedContexts.insert(context);

  auto top = context->topContext();
  if (allowedDepth >= 0) {
    foreach (Declaration* dec, context->localDeclarations(top)) {

      //IdentifiedType* idType = dynamic_cast<IdentifiedType*>(dec->abstractType().data());

      qout << Indent((m_indent+2) * 2) << "Declaration:" << dec->toString() << "[" << dec->qualifiedIdentifier() << "]"
        << dec << "(internal ctx:" << dec->internalContext() << ")" << dec->range().castToSimpleRange() << ","
        << (dec->isDefinition() ? "defined, " : (FunctionDefinition::definition(dec) ? "" : "no definition, "))
        << dec->uses().count() << "use(s)." << endl;
      if (FunctionDefinition::definition(dec)) {
        qout << Indent((m_indent+2) * 2 + 1) << "Definition:" << FunctionDefinition::definition(dec)->range().castToSimpleRange() << endl;
      }
      QMap<IndexedString, QList<RangeInRevision> > uses = dec->uses();
      for(QMap<IndexedString, QList<RangeInRevision> >::const_iterator it = uses.constBegin(); it != uses.constEnd(); ++it) {
        qout << Indent((m_indent+3) * 2) << "File:" << it.key().str() << endl;
        foreach (const RangeInRevision& range, *it)
          qout << Indent((m_indent+4) * 2) << "Use:" << range.castToSimpleRange() << endl;
      }
    }
  } else {
    qout << Indent((m_indent+2) * 2) << context->localDeclarations(top).count()
      << "Declarations," << context->childContexts().size() << "child-contexts" << endl;
  }

  ++m_indent;
  {
    /*
    foreach (const DUContext::Import &parent, context->importedParentContexts()) {
      DUContext* import = parent.context(top);
      if(!import) {
          qout << Indent((m_indent+2) * 2+1) << "Could not get parent, is it registered in the DUChain?" << endl;
          continue;
      }

      dump(import, allowedDepth-1, true, out);
    }
    */

    foreach (DUContext* child, context->childContexts())
      dump(child, allowedDepth-1, false, out);
  }
  --m_indent;
}

void DUChainDumper::dump(DUContext* context, int allowedDepth, QTextStream& out)
{
  d->m_visitedContexts.clear();

  if (!context) {
    out << "Error: Null context" << endl;
    return;
  }

  auto top = context->topContext();
  if (d->m_features.testFlag(DumpProblems)) {
    d->dumpProblems(top, out);
  }
  if (d->m_features.testFlag(DumpContext)) {
    d->dump(context, allowedDepth, false, out);
  }
}

void DUChainDumper::dump(DUContext* context, int allowedDepth)
{
  QTextStream out(stdout);
  dump(context, allowedDepth, out);
}
