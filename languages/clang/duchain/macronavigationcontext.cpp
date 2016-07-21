/*
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>
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

#include "macronavigationcontext.h"

#include "util/clangdebug.h"
#include "util/clangutils.h"

#include <KTextEditor/Editor>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KTextEditor/ConfigInterface>

#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <serialization/indexedstring.h>
#include <interfaces/icore.h>
#include <interfaces/ipartcontroller.h>

#include <QVBoxLayout>
#include <QLabel>

using namespace KDevelop;

MacroNavigationContext::MacroNavigationContext(const MacroDefinition::Ptr& macro, const KDevelop::DocumentCursor& expansionLocation)
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

QString MacroNavigationContext::html(bool shorten)
{
    clear();

    modifyHtml() += QLatin1String("<html><body><p>") + fontSizePrefix(shorten);
    addExternalHtml(m_prefix);

    QStringList parameterList;
    FOREACH_FUNCTION(const auto& parameter, m_macro->parameters) {
        parameterList << parameter.str();
    }
    const QString parameters = (!parameterList.isEmpty() ?
        QStringLiteral("(%1)").arg(parameterList.join(QLatin1String(", "))) :
        QString());

    const QUrl url = m_macro->url().toUrl();
    const QString path = url.toLocalFile();
    KTextEditor::Cursor cursor(m_macro->rangeInCurrentRevision().start());
    NavigationAction action(url, cursor);
    modifyHtml() += i18nc("%1: macro type, i.e.: 'Function macro' or just 'Macro'"
                          "%2: the macro name and arguments",
                          "%1: %2",
                          (m_macro->isFunctionLike() ? i18n("Function macro") : i18n("Macro")),
                          importantHighlight(name()) + parameters);
    modifyHtml() += QStringLiteral("<br/>");
    modifyHtml() += i18nc("%1: the link to the definition", "Defined in: %1",
                          createLink(QStringLiteral("%1 :%2").arg(url.fileName()).arg(cursor.line()+1), path, action));

    modifyHtml() += QStringLiteral(" "); //The action name _must_ stay "show_uses", since that is also used from outside
    makeLink(i18n("Show uses"), QStringLiteral("show_uses"), NavigationAction(m_macro.dynamicCast<Declaration>(), NavigationAction::NavigateUses));

    auto code = m_macro->definition().str();
    modifyHtml() += QLatin1String("<p>") + i18n("Body: ");
    modifyHtml() += QLatin1String("<tt>") + code.toHtmlEscaped().replace(QStringLiteral("\n"), QStringLiteral("<br/>")) + QLatin1String("</tt>");
    modifyHtml() += QLatin1String("</p>");

    modifyHtml() += fontSizeSuffix(shorten) + QLatin1String("</p></body></html>");
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
