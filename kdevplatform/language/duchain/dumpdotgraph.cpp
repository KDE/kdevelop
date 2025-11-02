/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "dumpdotgraph.h"

#include "ducontext.h"
#include "topducontext.h"
#include "declaration.h"
#include "duchainpointer.h"
#include "parsingenvironment.h"
#include "identifier.h"
#include "functiondefinition.h"

namespace KDevelop {
QString shortLabel(KDevelop::DUContext* context)
{
    return QStringLiteral("q%1").arg(( quint64 )context);
}

QString shortLabel(KDevelop::Declaration* declaration)
{
    return QStringLiteral("q%1").arg(( quint64 )declaration);
}

QString rangeToString(const KTextEditor::Range& r)
{
    return QStringLiteral("%1:%2->%3:%4").arg(r.start().line()).arg(r.start().column()).arg(r.end().line()).arg(
        r.end().column());
}

class DumpDotGraphPrivate
{
public:

    QString dotGraphInternal(KDevelop::DUContext* context, bool isMaster, bool shortened);

    void addDeclaration(QTextStream& stream, Declaration* decl);

    QMap<QUrl, QString> m_hadVersions;
    QMap<KDevelop::DUChainBase*, bool> m_hadObjects;
    TopDUContext* m_topContext;
};

QString DumpDotGraph::dotGraph(KDevelop::DUContext* context, bool shortened)
{
    Q_D(DumpDotGraph);

    d->m_hadObjects.clear();
    d->m_hadVersions.clear();
    d->m_topContext = context->topContext(); ///@todo maybe get this as a parameter
    return d->dotGraphInternal(context, true, shortened);
}

DumpDotGraph::DumpDotGraph()
    : d_ptr(new DumpDotGraphPrivate())
{
}

DumpDotGraph::~DumpDotGraph() = default;

void DumpDotGraphPrivate::addDeclaration(QTextStream& stream, Declaration* dec)
{
    if (m_hadObjects.contains(dec))
        return;

    m_hadObjects[dec] = true;

    Declaration* declarationForDefinition = nullptr;
    if (dynamic_cast<FunctionDefinition*>(dec))
        declarationForDefinition = static_cast<FunctionDefinition*>(dec)->declaration(m_topContext);

    if (!declarationForDefinition) {
        //Declaration
        stream << shortLabel(dec) <<
            "[shape=box, label=\"" <<
            dec->toString() << " [" <<
            dec->qualifiedIdentifier().toString() << "]" << " " <<
            rangeToString(dec->range().castToSimpleRange()) << "\"];\n";
        stream << shortLabel(dec->context()) << " -> " << shortLabel(dec) << "[color=green];\n";
        if (dec->internalContext())
            stream << shortLabel(dec) << " -> " << shortLabel(dec->internalContext()) <<
                "[label=\"internal\", color=blue];\n";
    } else {
        //Definition
        stream << shortLabel(dec) <<  "[shape=regular,color=yellow,label=\"" << declarationForDefinition->toString() <<
            " " << rangeToString(dec->range().castToSimpleRange()) <<  "\"];\n";
        stream << shortLabel(dec->context()) << " -> " << shortLabel(dec) << ";\n";

        stream << shortLabel(dec) << " -> " << shortLabel(declarationForDefinition) <<
            "[label=\"defines\",color=green];\n";
        addDeclaration(stream, declarationForDefinition);

        if (dec->internalContext())
            stream << shortLabel(dec) << " -> " << shortLabel(dec->internalContext()) <<
                "[label=\"internal\", color=blue];\n";
    }
}

QString DumpDotGraphPrivate::dotGraphInternal(KDevelop::DUContext* context, bool isMaster, bool shortened)
{
    if (m_hadObjects.contains(context))
        return QString();

    m_hadObjects[context] = true;

    QTextStream stream;
    QString ret;
    stream.setString(&ret, QIODevice::WriteOnly);

    if (isMaster)
        stream << "Digraph chain {\n";

    QString shape = QStringLiteral("parallelogram");
    //QString shape = "box";
    QString label = QStringLiteral("unknown");

    if (dynamic_cast<TopDUContext*>(context)) {
        auto* topCtx = static_cast<TopDUContext*>(context);
        if (topCtx->parsingEnvironmentFile()) {
            QUrl url = topCtx->parsingEnvironmentFile()->url().toUrl();
            const QString fileName = url.fileName();
            QString file = url.toDisplayString(QUrl::PreferLocalFile);
            if (topCtx->parsingEnvironmentFile() && topCtx->parsingEnvironmentFile()->isProxyContext())
                url.setPath(url.path() + QLatin1String("/_[proxy]_"));

            //Find the context this one is derived from. If there is one, connect it with a line, and shorten the url.
            if (m_hadVersions.contains(url)) {
                stream << shortLabel(context) << " -> " << m_hadVersions[url] << "[color=blue,label=\"version\"];\n";
                file = fileName;
            } else {
                m_hadVersions[url] = shortLabel(context);
            }

            label = file;

            if (topCtx->importers().count() != 0)
                label += QStringLiteral(" imported by %1").arg(topCtx->importers().count());
        } else {
            label = QStringLiteral("unknown file");
        }
        if (topCtx->parsingEnvironmentFile() && topCtx->parsingEnvironmentFile()->isProxyContext())
            label = QLatin1String("Proxy-context ") + label;
    } else {
        label = /*"context " + */ context->localScopeIdentifier().toString();
        label += QLatin1Char(' ') + rangeToString(context->range().castToSimpleRange());
    }

    //label = QStringLiteral("%1 ").arg((size_t)context) + label;

    if (isMaster && !dynamic_cast<TopDUContext*>(context)) {
        //Also draw contexts that import this one
        const auto importers = context->importers();
        for (DUContext* ctx : importers) {
            stream << dotGraphInternal(ctx, false, true);
        }
    }
    const auto importedParentContexts = context->importedParentContexts();
    for (const DUContext::Import& parent : importedParentContexts) {
        if (parent.context(m_topContext)) {
            stream << dotGraphInternal(parent.context(m_topContext), false, true);
            QString label = QStringLiteral("imports");
            if ((!dynamic_cast<TopDUContext*>(parent.context(m_topContext)) || !dynamic_cast<TopDUContext*>(context)) &&
                !(parent.context(m_topContext)->url() == context->url())) {
                label += QLatin1String(" from ") + parent.context(m_topContext)->url().toUrl().fileName()
                         + QLatin1String(" to ") + context->url().toUrl().fileName();
            }

            stream << shortLabel(context) << QLatin1String(" -> ") << shortLabel(parent.context(m_topContext)) <<
                QLatin1String("[style=dotted,label=\"") << label  << QLatin1String("\"];\n");
        }
    }

    const auto childContexts = context->childContexts();
    if (!childContexts.isEmpty()) {
        label += QStringLiteral(", %1 C.").arg(childContexts.count());
    }

    if (!shortened) {
        for (DUContext* child : childContexts) {
            stream << dotGraphInternal(child, false, false);
            stream << shortLabel(context) << QLatin1String(" -> ") << shortLabel(child) << QLatin1String(
                "[style=dotted,color=green];\n");
        }
    }

    const auto localDeclarations = context->localDeclarations();
    if (!localDeclarations.isEmpty()) {
        label += QStringLiteral(", %1 D.").arg(localDeclarations.count());
    }

    if (!shortened) {
        for (Declaration* dec : localDeclarations) {
            addDeclaration(stream, dec);
        }
    }

    if (context->owner()) {
        addDeclaration(stream, context->owner());
    }

    stream << shortLabel(context) << "[shape=" << shape << ",label=\"" << label << "\"" <<
    (isMaster ? QLatin1String("color=red") : QLatin1String("color=blue")) << "];\n";

    if (isMaster)
        stream << "}\n";

    return ret;
}
}
