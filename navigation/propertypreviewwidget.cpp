/*************************************************************************************
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


#include "propertypreviewwidget.h"

#include <QDeclarativeView>
#include <QLayout>
#include <qgraphicsitem.h>
#include <QLabel>
#include <KStandardDirs>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KLocalizedString>

// List of supported properties. The string must be the name of the property,
// which can contain dots if necessary
QHash<QString, SupportedProperty> PropertyPreviewWidget::supportedProperties;

QWidget* PropertyPreviewWidget::constructIfPossible(KTextEditor::Document* doc, SimpleRange keyRange,
                                                    SimpleRange valueRange, const QString& key, const QString& value)
{
    if ( supportedProperties.isEmpty() ) {
        KStandardDirs d;
        QStringList bases = d.findDirs("data", "propertywidgets");
        if ( bases.isEmpty() ) {
            return 0;
        }
        QString base = bases.first();
        supportedProperties["width"] = SupportedProperty(QUrl(base + "Width.qml"));
        supportedProperties["height"] = SupportedProperty(QUrl(base + "Height.qml"));
        supportedProperties["spacing"] = SupportedProperty(QUrl(base + "Spacing.qml"));
        supportedProperties["x"] = SupportedProperty(QUrl(base + "Distance.qml"));
        supportedProperties["y"] = SupportedProperty(QUrl(base + "Distance.qml"));
        // TODO support the other margins
        supportedProperties["Anchors.margins"] = SupportedProperty(QUrl(base + "Distance.qml"));
        supportedProperties["opacity"] = SupportedProperty(QUrl(base + "Opacity.qml"));
        supportedProperties["duration"] = SupportedProperty(QUrl(base + "Duration.qml"));
        supportedProperties["font.family"] = SupportedProperty(QUrl(base + "FontFamily.qml"));
        supportedProperties["font.pointSize"] = SupportedProperty(QUrl(base + "FontSize.qml"));
        supportedProperties["model"] = SupportedProperty(QUrl(base + "Repeat.qml"));
    }
    QHash<QString, SupportedProperty>::iterator item = supportedProperties.find(key);
    if ( item != supportedProperties.end() ) {
        return new PropertyPreviewWidget(doc, keyRange, valueRange, *item, value);
    }
    return 0;
}

void PropertyPreviewWidget::updateValue(const QString& newValue)
{
    if ( ! wasChanged ) {
        document->startEditing();
        wasChanged = true;
    }
    // communicate the changed value to the QML view
    view->rootObject()->setProperty("value", newValue);
    // set the cursor to the edited range, otherwise the view will jump if we call doc->endEditing()
    document->activeView()->setCursorPosition(KTextEditor::Cursor(valueRange.start.line, valueRange.start.column));
    if ( valueRange.end.column - valueRange.start.column == newValue.size() ) {
        document->replaceText(valueRange.textRange(), newValue);
    }
    else {
        // the length of the text changed so don't replace it but remove the old
        // and insert the new text.
        document->removeText(valueRange.textRange());
        document->insertText(valueRange.textRange().start(), newValue);
        valueRange.end.column = valueRange.start.column + newValue.size();
        document->endEditing();
        document->startEditing();
    }
}

PropertyPreviewWidget::~PropertyPreviewWidget()
{
    if ( wasChanged ) {
        document->endEditing();
    }
}

PropertyPreviewWidget::PropertyPreviewWidget(KTextEditor::Document* doc, SimpleRange keyRange, SimpleRange valueRange,
                                             const SupportedProperty& property, const QString& value)
    : QWidget()
    , view(new QDeclarativeView)
    , document(doc)
    , keyRange(keyRange)
    , valueRange(valueRange)
    , property(property)
    , wasChanged(false)
{
    // see docstring for ILanguageSupport::specialLanguageObjectNavigationWidget
    setProperty("DoNotCloseOnCursorMove", true);
    view->setSource(property.qmlfile);
    // don't crash because of a syntax error or missing QML file
    if ( ! view->rootObject() ) {
        return new QLabel(i18n("Error loading QML file:") + property.qmlfile.path());
    }
    // set the initial value read from the document
    view->rootObject()->setProperty("value", value);
    // connect to the slot which has to be emitted from QML when the value changes
    QObject::connect(view->rootObject(), SIGNAL(valueChanged(QString)),
                     this, SLOT(updateValue(QString)));
    setLayout(new QHBoxLayout);
    layout()->addWidget(view);
}
