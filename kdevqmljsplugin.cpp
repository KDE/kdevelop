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

int spacesAtCorner(const QString& string, int direction = +1) {
    int spaces = 0;
    QString::const_iterator it;
    for ( it = direction == 1 ? string.begin() : string.end()-1 ; it != string.end(); it += direction ) {
        if ( ! it->isSpace() ) break;
        spaces += 1;
    }
    return spaces;
}

const QPair<SimpleRange, SimpleRange> parseProperty(const QString& line, const SimpleCursor& position) {
    SimpleRange keyRange = SimpleRange(position, position);
    SimpleRange valueRange = SimpleRange(position, position);
    QStringList items = line.split(';');
    QString matchingItem;
    int col_offset = 0;
    foreach ( const QString& item, items ) {
        col_offset += item.size();
        if ( position.column < col_offset ) {
            matchingItem = item;
            break;
        }
    }
    QStringList split = matchingItem.split(':');
    if ( split.size() != 2 ) {
        return QPair<SimpleRange, SimpleRange>();
    }
    QString key = split.at(0);
    QString value = split.at(1);

    keyRange.start.column = col_offset - value.size() - key.size() + spacesAtCorner(key, +1) - 1;
    keyRange.end.column = col_offset - value.size() - 1 + spacesAtCorner(key, -1);
    valueRange.start.column = col_offset - value.size() + spacesAtCorner(value, +1);
    valueRange.end.column = col_offset + spacesAtCorner(value, -1);
    return QPair<SimpleRange, SimpleRange>(keyRange, valueRange);
};

SimpleRange KDevQmlJsPlugin::specialLanguageObjectRange(const KUrl& url, const SimpleCursor& position)
{
    IDocument* doc = ICore::self()->documentController()->documentForUrl(url);
    if ( doc && doc->textDocument() ) {
        const QString& line = doc->textDocument()->line(position.line);
        int start = position.column, end = position.column;
        for ( QString::const_iterator it = line.begin() + position.column; it != line.end(); it++ ){
            end += 1;
            if ( *it == '"' || *it == '\'' ) break;
        }
        for ( QString::const_iterator it = line.begin() + position.column; it != line.end(); it-- ){
            start -= 1;
            if ( *it == '"' || *it == '\'' ) break;
        }
        SimpleRange range(position.line, start+2, position.line, end-1);
        if ( range.isValid() && start+2 > 0 && stringToColor(textFromDoc(doc, range)).isValid() ) {
            return range;
        }
    }
    return KDevelop::ILanguageSupport::specialLanguageObjectRange(url, position);
}

QWidget* KDevQmlJsPlugin::specialLanguageObjectNavigationWidget(const KUrl& url, const SimpleCursor& position)
{
    SimpleRange range = specialLanguageObjectRange(url, position);
    IDocument* doc = ICore::self()->documentController()->documentForUrl(url);
    if ( doc && doc->textDocument() ) {
        QString text = textFromDoc(doc, range);
        QColor color = stringToColor(text);
        if ( color.isValid() ) {
            return new ColorChooser(color, doc->textDocument(), range);
        }

        QPair<SimpleRange, SimpleRange> property = parseProperty(doc->textDocument()->line(position.line), position);
        if ( property.first.isValid() && property.second.isValid() ) {
            return PropertyPreviewWidget::constructIfPossible(doc->textDocument(), property.first, property.second,
                                                              textFromDoc(doc, property.first), textFromDoc(doc, property.second));
        }
    }
    return KDevelop::ILanguageSupport::specialLanguageObjectNavigationWidget(url, position);
}


