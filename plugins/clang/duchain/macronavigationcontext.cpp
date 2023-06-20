/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "macronavigationcontext.h"

#include "util/clangdebug.h"
#include "util/clangutils.h"

#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>

using namespace KDevelop;

MacroNavigationContext::MacroNavigationContext(const MacroDefinition::Ptr& macro,
                                               const KDevelop::DocumentCursor& /* expansionLocation */)
    : m_macro(macro)
{
}

MacroNavigationContext::~MacroNavigationContext()
{
}

QString MacroNavigationContext::name() const
{
    return m_macro->identifier().toString();
}

static QString parameterListString(const MacroDefinition& macro)
{
    // A function-like macro can have 0 parameters, e.g. `#define m() 1`.
    // Check isFunctionLike() rather than parametersSize() here in order to display "()" in this case.
    if (!macro.isFunctionLike()) {
        Q_ASSERT(macro.parametersSize() == 0);
        return QString();
    }

    QStringList parameterList;
    parameterList.reserve(macro.parametersSize());
    FOREACH_FUNCTION (const auto& parameter, macro.parameters) {
        parameterList << parameter.str();
    }

    return QLatin1Char('(') + parameterList.join(QLatin1String(", ")) + QLatin1Char(')');
}

QString MacroNavigationContext::html(bool shorten)
{
    Q_UNUSED(shorten);
    clear();

    modifyHtml() += QStringLiteral("<html><body><p>");

    const QUrl url = m_macro->url().toUrl();
    const QString path = url.toLocalFile();
    KTextEditor::Cursor cursor(m_macro->rangeInCurrentRevision().start());
    NavigationAction action(url, cursor);
    modifyHtml() += i18nc("%1: macro type, i.e.: 'Function macro' or just 'Macro'"
                          "%2: the macro name and arguments",
                          "%1: %2",
                          (m_macro->isFunctionLike() ? i18n("Function macro") : i18n("Macro")),
                          importantHighlight(name()) + parameterListString(*m_macro));
    modifyHtml() += QStringLiteral("<br/>");
    modifyHtml() += i18nc("%1: the link to the definition", "Defined in: %1",
                          createLink(QStringLiteral("%1 :%2").arg(url.fileName()).arg(cursor.line()+1), path, action));

    modifyHtml() += QStringLiteral(" "); //The action name _must_ stay "show_uses", since that is also used from outside
    makeLink(i18n("Show uses"), QStringLiteral("show_uses"), NavigationAction(m_macro.dynamicCast<Declaration>(), NavigationAction::NavigateUses));

    auto code = m_macro->definition().str();
    modifyHtml() += QLatin1String("<p>") + i18n("Body: ");
    modifyHtml() += QLatin1String("<tt>") + code.toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br/>")) + QLatin1String("</tt>");
    modifyHtml() += QStringLiteral("</p>");

    modifyHtml() += QStringLiteral("</p></body></html>");
    return currentHtml();
}

QString MacroNavigationContext::retrievePreprocessedBody(const DocumentCursor& /*expansionLocation*/) const
{
    const TopDUContext* topContext = m_macro->topContext();
    if (!topContext) {
        return QString();
    }

    // TODO: Implement me. Still not exactly sure what do to here...
    return QString();
}

#include "moc_macronavigationcontext.cpp"
