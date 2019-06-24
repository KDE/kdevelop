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

#include "abstractincludenavigationcontext.h"

#include <KLocalizedString>

#include <language/duchain/duchain.h>
#include <language/duchain/parsingenvironment.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>

namespace KDevelop {
AbstractIncludeNavigationContext::AbstractIncludeNavigationContext(const IncludeItem& item,
                                                                   const TopDUContextPointer& topContext,
                                                                   const ParsingEnvironmentType& type)
    : AbstractNavigationContext(topContext)
    , m_type(type)
    , m_item(item)
{}

TopDUContext* pickContextWithData(const QList<TopDUContext*>& duchains, uint maxDepth,
                                  const ParsingEnvironmentType& type,
                                  bool forcePick = true)
{
    TopDUContext* duchain = nullptr;

    for (TopDUContext* ctx : duchains) {
        if (!ctx->parsingEnvironmentFile() || ctx->parsingEnvironmentFile()->type() != type)
            continue;

        if (ctx->childContexts().count() != 0
            && (duchain == nullptr || ctx->childContexts().count() > duchain->childContexts().count())) {
            duchain = ctx;
        }
        if (ctx->localDeclarations().count() != 0
            && (duchain == nullptr || ctx->localDeclarations().count() > duchain->localDeclarations().count())) {
            duchain = ctx;
        }
    }

    if (!duchain && maxDepth != 0) {
        if (maxDepth != 0) {
            for (TopDUContext* ctx : duchains) {
                QList<TopDUContext*> children;
                const auto importedParentContexts = ctx->importedParentContexts();
                for (const DUContext::Import& import : importedParentContexts) {
                    if (import.context(nullptr))
                        children << import.context(nullptr)->topContext();
                }

                duchain = pickContextWithData(children, maxDepth - 1, type, false);
                if (duchain)
                    break;
            }
        }
    }

    if (!duchain && !duchains.isEmpty() && forcePick)
        duchain = duchains.first();

    return duchain;
}

QString AbstractIncludeNavigationContext::html(bool shorten)
{
    clear();
    modifyHtml()  += QLatin1String("<html><body><p>");

    QUrl u = m_item.url();
    NavigationAction action(u, KTextEditor::Cursor(0, 0));
    makeLink(u.toDisplayString(QUrl::PreferLocalFile), u.toString(), action);
    modifyHtml() += QStringLiteral("<br />");

    DUChainReadLocker lock;
    QList<TopDUContext*> duchains = DUChain::self()->chainsForDocument(u);
    //Pick the one duchain for this document that has the most child-contexts/declarations.
    //This prevents picking a context that is empty due to header-guards.
    TopDUContext* duchain = pickContextWithData(duchains, 2, m_type);

    if (duchain) {
        getFileInfo(duchain);
        if (!shorten) {
            modifyHtml() += labelHighlight(i18n("Declarations:")) + QLatin1String("<br />");
            bool first = true;
            QVector<IdentifierPair> decs;
            addDeclarationsFromContext(duchain, first, decs);
        }
    } else if (duchains.isEmpty()) {
        modifyHtml() += i18n("not parsed yet");
    }

    modifyHtml() += QLatin1String("</p></body></html>");
    return currentHtml();
}

void AbstractIncludeNavigationContext::getFileInfo(TopDUContext* duchain)
{
    modifyHtml() +=
        QStringLiteral("%1: %2 %3: %4").arg(labelHighlight(i18nc("Files included into this file", "Includes"))).arg(
            duchain->importedParentContexts().count()).arg(labelHighlight(i18nc(
                                                                              "Count of files this file was included into",
                                                                              "Included by"))).arg(
            duchain->importers().count());
    modifyHtml() += QStringLiteral("<br />");
}

QString AbstractIncludeNavigationContext::name() const
{
    return m_item.name;
}

bool AbstractIncludeNavigationContext::filterDeclaration(Declaration* /*decl*/)
{
    return true;
}

void AbstractIncludeNavigationContext::addDeclarationsFromContext(KDevelop::DUContext* ctx, bool& first,
                                                                  QVector<IdentifierPair>& addedDeclarations,
                                                                  const QString& indent)
{
    //modifyHtml() += indent + ctx->localScopeIdentifier().toString() + "{<br />";
    QVector<DUContext*> children = ctx->childContexts();
    QVector<Declaration*> declarations = ctx->localDeclarations();

    QVector<DUContext*>::const_iterator childIterator = children.constBegin();
    QVector<Declaration*>::const_iterator declarationIterator = declarations.constBegin();

    while (childIterator != children.constEnd() || declarationIterator != declarations.constEnd()) {
        //Show declarations/contexts in the order they appear in the file
        int currentDeclarationLine = -1;
        int currentContextLine = -1;
        if (declarationIterator != declarations.constEnd())
            currentDeclarationLine = (*declarationIterator)->rangeInCurrentRevision().start().line();

        if (childIterator != children.constEnd())
            currentDeclarationLine = (*childIterator)->rangeInCurrentRevision().start().line();

        if ((currentDeclarationLine <= currentContextLine || currentContextLine == -1 ||
             childIterator == children.constEnd()) && declarationIterator != declarations.constEnd()) {
            IdentifierPair id = qMakePair(static_cast<int>((*declarationIterator)->kind()),
                                          (*declarationIterator)->qualifiedIdentifier().index());
            if (!addedDeclarations.contains(id) && filterDeclaration(*declarationIterator)) {
                //Show the declaration
                if (!first)
                    modifyHtml() += QStringLiteral(", ");
                else
                    first = false;

                modifyHtml() += QString(indent + declarationKind(DeclarationPointer(
                                                                     *declarationIterator)) +
                                        QLatin1Char(' ')).toHtmlEscaped();
                makeLink((*declarationIterator)->qualifiedIdentifier().toString(),
                         DeclarationPointer(*declarationIterator), NavigationAction::NavigateDeclaration);

                addedDeclarations << id;
            }
            ++declarationIterator;
        } else {
            //Eventually Recurse into the context
            if ((*childIterator)->type() == DUContext::Global ||
                (*childIterator)->type() == DUContext::Namespace /*|| (*childIterator)->type() == DUContext::Class*/)
                addDeclarationsFromContext(*childIterator, first, addedDeclarations, indent + QLatin1Char(' '));
            ++childIterator;
        }
    }
    //modifyHtml() += "}<br />";
}
}
