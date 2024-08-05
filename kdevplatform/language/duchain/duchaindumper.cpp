/*
    SPDX-FileCopyrightText: 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "duchaindumper.h"

#include <QString>
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
#include <editor/documentrange.h>

namespace {
QDebug fromTextStream(const QTextStream& out)
{
    if (out.device()) {
        return QDebug{out.device()};
    }
    return QDebug{out.string()};
}
}

namespace KDevelop {
QString typeToString(DUContext::ContextType type)
{
    switch (type) {
    case DUContext::Global: return QStringLiteral("Global");
    case DUContext::Namespace: return QStringLiteral("Namespace");
    case DUContext::Class: return QStringLiteral("Class");
    case DUContext::Function: return QStringLiteral("Function");
    case DUContext::Template: return QStringLiteral("Template");
    case DUContext::Enum: return QStringLiteral("Enum");
    case DUContext::Helper: return QStringLiteral("Helper");
    case DUContext::Other: return QStringLiteral("Other");
    }
    Q_ASSERT(false);
    return QString();
}

class DUChainDumperPrivate
{
public:
    DUChainDumperPrivate()
        : m_indent(0)
    {}

    void dumpProblems(TopDUContext* top, QTextStream& out);
    void dump(DUContext* context, int allowedDepth, bool isFromImport, QTextStream& out);

    int m_indent;
    DUChainDumper::Features m_features;
    QSet<DUContext*> m_visitedContexts;
};

DUChainDumper::DUChainDumper(Features features)
    : d_ptr(new DUChainDumperPrivate)
{
    Q_D(DUChainDumper);

    d->m_features = features;
}

DUChainDumper::~DUChainDumper()
{
}

class Indent
{
public:
    explicit Indent(int level) : m_level(level) {}

    friend QDebug& operator<<(QDebug& debug, const Indent& ind)
    {
        for (int i = 0; i < ind.m_level; i++) {
            debug.nospace() << ' ';
        }

        return debug.space();
    }

private:
    int m_level;
};

void DUChainDumperPrivate::dumpProblems(TopDUContext* top, QTextStream& out)
{
    QDebug qout = fromTextStream(out);

    if (!top->problems().isEmpty()) {
        qout << top->problems().size() << "problems encountered:" << Qt::endl;
        const auto problems = top->problems();
        for (const ProblemPointer& p : problems) {
            qout << Indent(m_indent * 2) << p->description() << p->explanation() << p->finalLocation() << Qt::endl;
        }

        qout << Qt::endl;
    }
}

void DUChainDumperPrivate::dump(DUContext* context, int allowedDepth, bool isFromImport, QTextStream& out)
{
    QDebug qout = fromTextStream(out);

    qout << Indent(m_indent * 2) << (isFromImport ? " ==import==>" : "")
         << (dynamic_cast<TopDUContext*>(context) ? "Top-Context" : "Context") << typeToString(context->type())
         << "(owner: " << context->owner() << ")" << context << context->localScopeIdentifier() << "["
         << context->scopeIdentifier(true) << "]" << context->range().castToSimpleRange()
         << (dynamic_cast<TopDUContext*>(context) ? static_cast<TopDUContext*>(context)->url().byteArray() : "")
         << Qt::endl;

    if (m_visitedContexts.contains(context)) {
        qout << Indent((m_indent + 2) * 2) << "(Skipping" << context->scopeIdentifier(true)
             << "because it was already printed)" << Qt::endl;
        return;
    }

    m_visitedContexts.insert(context);

    auto top = context->topContext();
    if (allowedDepth >= 0) {
        const auto localDeclarations = context->localDeclarations(top);
        for (Declaration* dec : localDeclarations) {
            //IdentifiedType* idType = dynamic_cast<IdentifiedType*>(dec->abstractType().data());

            qout << Indent((m_indent + 2) * 2) << "Declaration:" << dec->toString() << "[" << dec->qualifiedIdentifier()
                 << "]" << dec << "(internal ctx:" << dec->internalContext() << ")" << dec->range().castToSimpleRange()
                 << ","
                 << (dec->isDefinition() ? "defined, " : (FunctionDefinition::definition(dec) ? "" : "no definition, "))
                 << dec->uses().count() << "use(s)." << Qt::endl;
            if (FunctionDefinition::definition(dec)) {
                qout << Indent((m_indent + 2) * 2 + 1)
                     << "Definition:" << FunctionDefinition::definition(dec)->range().castToSimpleRange() << Qt::endl;
            }
            const auto uses = dec->uses();
            for (auto it = uses.constBegin(); it != uses.constEnd(); ++it) {
                qout << Indent((m_indent + 3) * 2) << "File:" << it.key().str() << Qt::endl;
                for (const RangeInRevision range : std::as_const(*it)) {
                    qout << Indent((m_indent + 4) * 2) << "Use:" << range.castToSimpleRange() << Qt::endl;
                }
            }
        }
    } else {
        qout << Indent((m_indent + 2) * 2) << context->localDeclarations(top).count() << "Declarations,"
             << context->childContexts().size() << "child-contexts" << Qt::endl;
    }

    ++m_indent;
    {
        /*
           const auto importedParentContexts = context->importedParentContexts();
           for (const DUContext::Import& parent : importedParentContexts) {
           DUContext* import = parent.context(top);
           if(!import) {
              qout << Indent((m_indent+2) * 2+1) << "Could not get parent, is it registered in the DUChain?" << Qt::endl;
              continue;
           }

           dump(import, allowedDepth-1, true, out);
           }
         */

        const auto childContexts = context->childContexts();
        for (DUContext* child : childContexts) {
            dump(child, allowedDepth - 1, false, out);
        }
    }
    --m_indent;
}

void DUChainDumper::dump(DUContext* context, int allowedDepth, QTextStream& out)
{
    Q_D(DUChainDumper);

    d->m_visitedContexts.clear();

    if (!context) {
        out << "Error: Null context" << Qt::endl;
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

}
