/*************************************************************************************
 *  Copyright (C) 2012 by Aleix Pol <aleixpol@kde.org>                               *
 *  Copyright (C) 2012 by Milian Wolff <mail@milianw.de>                             *
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
#include "version.h"
#include "codecompletion/model.h"

#include "navigation/colorchooser.h"
#include "navigation/propertypreviewwidget.h"

#include <KPluginFactory>
#include <KAboutData>

#include <language/codecompletion/codecompletion.h>
#include <language/duchain/duchain.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>

K_PLUGIN_FACTORY(KDevQmlJsSupportFactory, registerPlugin<KDevQmlJsPlugin>(); )
K_EXPORT_PLUGIN(KDevQmlJsSupportFactory(
    KAboutData("kdevqmljssupport", 0, ki18n("QML/JS Support"), VERSION_STR,
    ki18n("Support for QML and JS Languages"), KAboutData::License_GPL)))

using namespace KDevelop;

KDevQmlJsPlugin::KDevQmlJsPlugin(QObject* parent, const QVariantList& )
: IPlugin( KDevQmlJsSupportFactory::componentData(), parent )
, ILanguageSupport()
, m_highlighting(new QmlJsHighlighting(this))
{
    KDEV_USE_EXTENSION_INTERFACE(ILanguageSupport)

    CodeCompletionModel* codeCompletion = new QmlJS::CodeCompletionModel(this);
    new KDevelop::CodeCompletion(this, codeCompletion, "qml/js");
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

const QColor stringToColor(const QString& text) {
    if ( text.size() < 20 ) {
        QColor color;
        // TODO handle alpha colors, this can't
        color.setNamedColor(text);
        if ( color.isValid() ) {
            qDebug() << color;
            return color;
        }
    }
    return QColor();
}

const QString textFromDoc(const IDocument* doc, const SimpleRange& range) {
    return doc->textDocument()->line(range.start.line).mid(range.start.column, range.end.column-range.start.column);
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
const QPair<SimpleRange, SimpleRange> parseProperty(const QString& line, const SimpleCursor& position) {
    SimpleRange keyRange = SimpleRange(position, position);
    SimpleRange valueRange = SimpleRange(position, position);
    QStringList items = line.split(';');
    QString matchingItem;
    int col_offset = -1;
    // This is to also support FooAnimation { foo: bar; baz: bang; duration: 200 }
    // or similar
    foreach ( const QString& item, items ) {
        col_offset += item.size() + 1;
        if ( position.column < col_offset ) {
            matchingItem = item;
            break;
        }
    }
    QStringList split = matchingItem.split(':');
    if ( split.size() != 2 ) {
        // The expression is not of the form foo:bar, thus invalid.
        return QPair<SimpleRange, SimpleRange>();
    }
    QString key = split.at(0);
    QString value = split.at(1);

    // For animations or similar, remove the trailing '}' if there's no semicolon after the last entry
    if ( value.trimmed().endsWith('}') ) {
        col_offset -= value.size() - value.lastIndexOf('}') + 1;
        value = value.left(value.lastIndexOf('}')-1);
    }

    keyRange.start.column = col_offset - value.size() - key.size() + spacesAtCorner(key, +1) - 1;
    keyRange.end.column = col_offset - value.size() - 1 + spacesAtCorner(key, -1);
    valueRange.start.column = col_offset - value.size() + spacesAtCorner(value, +1);
    valueRange.end.column = col_offset + spacesAtCorner(value, -1);
    return QPair<SimpleRange, SimpleRange>(keyRange, valueRange);
};

SimpleRange KDevQmlJsPlugin::specialLanguageObjectRange(const KUrl& url, const SimpleCursor& position)
{
    // The object range is only determined for colors. There is not really a point of doing this for
    // the key/value stuff; it'll just highlight the whole line, which is more annoying
    // than useful.
    IDocument* doc = ICore::self()->documentController()->documentForUrl(url);
    if ( doc && doc->textDocument() ) {
        const QString& line = doc->textDocument()->line(position.line);
        int start = position.column, end = position.column;
        // search for the " or ' delmiters of the color to the right and to the left
        for ( QString::const_iterator it = line.begin() + position.column; it != line.end(); it++ ){
            end += 1;
            if ( *it == '"' || *it == '\'' ) break;
        }
        for ( QString::const_iterator it = line.begin() + position.column; it != line.end(); it-- ){
            start -= 1;
            if ( *it == '"' || *it == '\'' ) break;
        }
        SimpleRange range(position.line, start+2, position.line, end-1);
        // check if the encompassed string is actually a valid color string (this also
        // matches "red" or "blue") and if yes, return its range for highlighting.
        if ( range.isValid() && start+2 > 0 && stringToColor(textFromDoc(doc, range)).isValid() ) {
            return range;
        }
    }
    // Otherwise, no special highlighting shall take place.
    return KDevelop::ILanguageSupport::specialLanguageObjectRange(url, position);
}

QWidget* KDevQmlJsPlugin::specialLanguageObjectNavigationWidget(const KUrl& url, const SimpleCursor& position)
{
    SimpleRange range = specialLanguageObjectRange(url, position);
    IDocument* doc = ICore::self()->documentController()->documentForUrl(url);
    if ( doc && doc->textDocument() ) {
        // use the above function to check if the requested range contains a color,
        // and if yes, return a color picker widget to display.
        QString text = textFromDoc(doc, range);
        QColor color = stringToColor(text);
        if ( color.isValid() ) {
            return new ColorChooser(color, doc->textDocument(), range);
        }

        // If it's not a color, check for a QML property, and construct a property preview widget
        // if the property key is listed in the supported properties.
        QPair<SimpleRange, SimpleRange> property = parseProperty(doc->textDocument()->line(position.line), position);
        if ( property.first.isValid() && property.second.isValid() ) {
            return PropertyPreviewWidget::constructIfPossible(doc->textDocument(), property.first, property.second,
                                                              textFromDoc(doc, property.first), textFromDoc(doc, property.second));
        }
    }
    // Otherwise, display no special "navigation" widget.
    return KDevelop::ILanguageSupport::specialLanguageObjectNavigationWidget(url, position);
}


