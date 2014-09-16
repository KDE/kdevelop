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

#include <KLocale>

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

namespace {

KTextEditor::View* createDocAndView(const QString& data, KTextEditor::Document** docPtr)
{
    if (data.isEmpty()) {
        return 0;
    }

    KTextEditor::Document* doc = ICore::self()->partController()->editorPart()->createDocument(0);
    *docPtr = doc;
    doc->setText(data);
    doc->setMode("C++");
    doc->setReadWrite(false);

    KTextEditor::View* view = doc->createView(0);
    view->setStatusBarEnabled(false);
    if (KTextEditor::ConfigInterface* config = qobject_cast<KTextEditor::ConfigInterface*>(view)) {
        config->setConfigValue("icon-bar", false);
        config->setConfigValue("folding-bar", false);
        config->setConfigValue("line-numbers", false);
        config->setConfigValue("dynamic-word-wrap", true);
    }
    return view;
}

}

MacroNavigationContext::MacroNavigationContext(const MacroDefinition::Ptr& macro, const KDevelop::DocumentCursor& expansionLocation)
    : m_macro(macro)
    , m_preprocessed(nullptr)
    , m_definition(nullptr)
    , m_widget(new QWidget)
{
    QVBoxLayout* layout = new QVBoxLayout(m_widget.data());

    if (expansionLocation.isValid()) {
        const QString preprocessedBody = retrievePreprocessedBody(expansionLocation);
        KTextEditor::View* preprocessedView = createDocAndView(preprocessedBody, &m_preprocessed);
        if (m_preprocessed) {
            layout->addWidget(new QLabel(i18n("Preprocessed Body:")));
            layout->addWidget(preprocessedView);
        } else {
            layout->addWidget(new QLabel(i18n("Preprocessed Body: (empty)")));
        }
    }

    const QString definitionText = m_macro->definition().str();
    KTextEditor::View* definitionView = createDocAndView(definitionText, &m_definition);
    if (m_definition) {
        layout->addWidget(new QLabel(i18n("Body:")));
        layout->addWidget(definitionView);
    } else {
        layout->addWidget(new QLabel(i18n("Body: (empty)")));
    }
    m_widget->setLayout(layout);
}

MacroNavigationContext::~MacroNavigationContext()
{
    delete m_preprocessed;
    delete m_definition;
    delete m_widget;
}

QString MacroNavigationContext::name() const
{
    return m_macro->identifier().toString();
}

QWidget* MacroNavigationContext::widget() const
{
    return m_widget.data();
}

QString MacroNavigationContext::html(bool shorten)
{
    clear();

    modifyHtml() += "<html><body><p>" + fontSizePrefix(shorten);
    addExternalHtml(m_prefix);

    QStringList parameterList;
    FOREACH_FUNCTION(const auto& parameter, m_macro->parameters) {
        parameterList << parameter.str();
    }
    const QString parameters = (!parameterList.isEmpty() ?
        QString("(%1)").arg(parameterList.join(", ")) :
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
    modifyHtml() += "<br/>";
    modifyHtml() += i18nc("%1: the link to the definition", "Defined in: %1",
                          createLink(QString("%1 :%2").arg(url.fileName()).arg(cursor.line()+1), path, action));

    modifyHtml() += " "; //The action name _must_ stay "show_uses", since that is also used from outside
    makeLink(i18n("Show uses"), "show_uses", NavigationAction(m_macro.dynamicCast<Declaration>(), NavigationAction::NavigateUses));

    modifyHtml() += fontSizeSuffix(shorten) + "</p></body></html>";
    return currentHtml();
}

QString MacroNavigationContext::retrievePreprocessedBody(const DocumentCursor& expansionLocation) const
{
    const TopDUContext* topContext = m_macro->topContext();
    if (!topContext) {
        return QString();
    }

    // TODO: Implement me. Still not exactly sure what do to here...
    return QString();
}
