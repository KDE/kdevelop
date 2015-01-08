/*************************************************************************************
 *  Copyright (C) 2012 by Aleix Pol <aleixpol@kde.org>                               *
 *  Copyright (C) 2012 by Milian Wolff <mail@milianw.de>                             *
 *  Copyright (C) 2013 by Sven Brauch <svenbrauch@gmail.com>                         *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "kdevqmljsplugin.h"

#include "qmljsparsejob.h"
#include "qmljshighlighting.h"
#include "codecompletion/model.h"
#include "navigation/propertypreviewwidget.h"

#include <KPluginFactory>
#include <KAboutData>

#include <language/codecompletion/codecompletion.h>
#include <language/assistant/renameassistant.h>
#include <language/assistant/staticassistantsmanager.h>
#include <language/codegen/basicrefactoring.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainutils.h>
#include <language/interfaces/editorcontext.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/contextmenuextension.h>

K_PLUGIN_FACTORY(KDevQmlJsSupportFactory, registerPlugin<KDevQmlJsPlugin>(); )
//K_EXPORT_PLUGIN(KDevQmlJsSupportFactory(
//    KAboutData("kdevqmljssupport", 0, ki18n("QML/JS Support"), VERSION_STR,
//    ki18n("Support for QML and JS Languages"), KAboutData::License_GPL)))

using namespace KDevelop;

KDevQmlJsPlugin::KDevQmlJsPlugin(QObject* parent, const QVariantList& )
: IPlugin(QLatin1String("kdevqmljssupport"), parent )
, ILanguageSupport()
, m_highlighting(new QmlJsHighlighting(this))
, m_refactoring(new BasicRefactoring(this))
{
    KDEV_USE_EXTENSION_INTERFACE(ILanguageSupport)

    CodeCompletionModel* codeCompletion = new QmlJS::CodeCompletionModel(this);
    new KDevelop::CodeCompletion(this, codeCompletion, name());

    auto assistantsManager = core()->languageController()->staticAssistantsManager();
    assistantsManager->registerAssistant(StaticAssistant::Ptr(new RenameAssistant(this)));
}

KDevQmlJsPlugin::~KDevQmlJsPlugin()
{
}

ParseJob* KDevQmlJsPlugin::createParseJob(const IndexedString& url)
{
    return new QmlJsParseJob(url, this);
}

QString KDevQmlJsPlugin::name() const
{
    return "qml/js";
}

ICodeHighlighting* KDevQmlJsPlugin::codeHighlighting() const
{
    return m_highlighting;
}

BasicRefactoring* KDevQmlJsPlugin::refactoring() const
{
    return m_refactoring;
}

ContextMenuExtension KDevQmlJsPlugin::contextMenuExtension(Context* context)
{
    ContextMenuExtension cm;
    EditorContext *ec = dynamic_cast<KDevelop::EditorContext *>(context);

    if (ec && ICore::self()->languageController()->languagesForUrl(ec->url()).contains(this)) {
        // It's a QML/JS file, let's add our context menu.
        m_refactoring->fillContextMenu(cm, context);
    }

    return cm;
}

const QString textFromDoc(const IDocument* doc, const KTextEditor::Range& range) {
    return doc->textDocument()->line(range.start().line()).mid(range.start().column(), range.end().column()-range.start().column());
};

// Finds how many spaces the given string has at one end.
// direction=+1 -> left end of the string, -1 for right end.
int spacesAtCorner(const QString& string, int direction = +1) {
    int spaces = 0;
    QString::const_iterator it;
    for ( it = direction == 1 ? string.begin() : string.end()-1 ; it != string.end(); it += direction ) {
        if ( ! it->isSpace() ) break;
        spaces += 1;
    }
    return spaces;
}

// Take the given QML line and check if it's a line of the form foo.bar: value.
// Return ranges for the key and the value.
const QPair<KTextEditor::Range, KTextEditor::Range> parseProperty(const QString& line, const KTextEditor::Cursor& position) {
    QStringList items = line.split(';');
    QString matchingItem;
    int col_offset = -1;
    // This is to also support FooAnimation { foo: bar; baz: bang; duration: 200 }
    // or similar
    foreach ( const QString& item, items ) {
        col_offset += item.size() + 1;
        if ( position.column() < col_offset ) {
            matchingItem = item;
            break;
        }
    }
    QStringList split = matchingItem.split(':');
    if ( split.size() != 2 ) {
        // The expression is not of the form foo:bar, thus invalid.
        return qMakePair(KTextEditor::Range::invalid(), KTextEditor::Range::invalid());
    }
    QString key = split.at(0);
    QString value = split.at(1);

    // For animations or similar, remove the trailing '}' if there's no semicolon after the last entry
    if ( value.trimmed().endsWith('}') ) {
        col_offset -= value.size() - value.lastIndexOf('}') + 1;
        value = value.left(value.lastIndexOf('}')-1);
    }

    return qMakePair(
    KTextEditor::Range(
        KTextEditor::Cursor(position.line(), col_offset - value.size() - key.size() + spacesAtCorner(key, +1) - 1),
        KTextEditor::Cursor(position.line(), col_offset - value.size() - 1 + spacesAtCorner(key, -1))
    ),
    KTextEditor::Range(
        KTextEditor::Cursor(position.line(), col_offset - value.size() + spacesAtCorner(value, +1)),
        KTextEditor::Cursor(position.line(), col_offset + spacesAtCorner(value, -1))
    ));
};

QWidget* KDevQmlJsPlugin::specialLanguageObjectNavigationWidget(const QUrl& url, const KTextEditor::Cursor& position)
{
    IDocument* doc = ICore::self()->documentController()->documentForUrl(url);
    if ( doc && doc->textDocument() ) {
        // Check for a QML property, and construct a property preview widget
        // if the property key is listed in the supported properties.
        QPair<KTextEditor::Range, KTextEditor::Range> property = parseProperty(doc->textDocument()->line(position.line()), position);
        if ( property.first.isValid() && property.second.isValid() ) {
            Declaration* decl = DUChainUtils::itemUnderCursor(url, property.first.start());

            return PropertyPreviewWidget::constructIfPossible(
                doc->textDocument(),
                property.first,
                property.second,
                decl,
                textFromDoc(doc, property.first),
                textFromDoc(doc, property.second)
            );
        }
    }
    // Otherwise, display no special "navigation" widget.
    return KDevelop::ILanguageSupport::specialLanguageObjectNavigationWidget(url, position);
}

#include "kdevqmljsplugin.moc"
