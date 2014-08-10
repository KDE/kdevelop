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

#include <QQuickWidget>
#include <QQuickItem>
#include <QLayout>
#include <QLabel>
#include <KStandardDirs>
#include <KLocalizedString>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KDeclarative/KDeclarative>

#include <language/duchain/ducontext.h>
#include <language/duchain/duchainlock.h>

// List of supported properties. The string must be the name of the property,
// which can contain dots if necessary
QHash<QString, SupportedProperty> PropertyPreviewWidget::supportedProperties;

QWidget* PropertyPreviewWidget::constructIfPossible(KTextEditor::Document* doc,
                                                    KTextEditor::Range keyRange,
                                                    KTextEditor::Range valueRange,
                                                    Declaration* decl,
                                                    const QString& key,
                                                    const QString& value)
{
#define PROP(key, filename, type, class) \
    supportedProperties.insertMulti(key, SupportedProperty(QUrl(base + filename), type, class));

    if ( supportedProperties.isEmpty() ) {
        KStandardDirs d;
        QStringList bases = d.findDirs("data", "propertywidgets");
        if ( bases.isEmpty() ) {
            return 0;
        }
        QString base = bases.first();

        // Positioning
        PROP("width", "Width.qml", QString(), QString())
        PROP("height", "Height.qml", QString(), QString())
        PROP("spacing", "Spacing.qml", QString(), QString())

        // Margins
        PROP("margins", "Spacing.qml", QString(), "QQuickAnchors");         // matches anchors.margins and anchors { margins: }
        PROP("margins", "Spacing.qml", QString(), "QDeclarativeAnchors");
        PROP("leftMargin", "Spacing.qml", QString(), "QQuickAnchors");
        PROP("leftMargin", "Spacing.qml", QString(), "QDeclarativeAnchors");
        PROP("rightMargin", "Spacing.qml", QString(), "QQuickAnchors");
        PROP("rightMargin", "Spacing.qml", QString(), "QDeclarativeAnchors");
        PROP("topMargin", "Spacing.qml", QString(), "QQuickAnchors");
        PROP("topMargin", "Spacing.qml", QString(), "QDeclarativeAnchors");
        PROP("bottomMargin", "Spacing.qml", QString(), "QQuickAnchors");
        PROP("bottomMargin", "Spacing.qml", QString(), "QDeclarativeAnchors");

        // Animations
        PROP("duration", "Duration.qml", QString(), QString())

        // Font QDeclarativeFontValueType, QQuickFontValueType
        PROP("family", "FontFamily.qml", QString(), "QDeclarativeFontValueType")
        PROP("family", "FontFamily.qml", QString(), "QQuickFontValueType")
        PROP("pointSize", "FontSize.qml", QString(), "QDeclarativeFontValueType")
        PROP("pointSize", "FontSize.qml", QString(), "QQuickFontValueType")

        // Appearance
        PROP("opacity", "Opacity.qml", QString(), QString())

        // Type-dependend widgets
        PROP(QString(), "ColorPicker.qml", "color", QString())
    }
#undef PROP

    QList<SupportedProperty> properties;

    properties << supportedProperties.values(key.section(QLatin1Char('.'), -1, -1));
    properties << supportedProperties.values(QString());

    // Explore each possible supported property and return the first supported widget
    DUChainReadLocker lock;

    for (const SupportedProperty& property : properties) {
        if (!decl || !decl->abstractType() || !decl->context() || !decl->context()->owner()) {
            continue;
        }

        if (!decl->abstractType()->toString().contains(property.typeContains)) {
            continue;
        }

        if (!decl->context()->owner()->toString().contains(property.classContains)) {
            continue;
        }

        return new PropertyPreviewWidget(doc, keyRange, valueRange, property, value);
    }

    return 0;
}

void PropertyPreviewWidget::updateValue(const QString& newValue)
{
    // communicate the changed value to the QML view
    view->rootObject()->setProperty("value", newValue);
    // set the cursor to the edited range, otherwise the view will jump if we call doc->endEditing()
    //document->activeView()->setCursorPosition(KTextEditor::Cursor(valueRange.start.line, valueRange.start.column));
    if (valueRange.end().column() - valueRange.start().column() == newValue.size()) {
        document->replaceText(valueRange, newValue);
    }
    else {
        // the length of the text changed so don't replace it but remove the old
        // and insert the new text.
        document->removeText(valueRange);
        document->insertText(valueRange.start(), newValue);
        valueRange.end().setColumn(valueRange.start().column() + newValue.size());
    }
}

PropertyPreviewWidget::~PropertyPreviewWidget()
{
}

PropertyPreviewWidget::PropertyPreviewWidget(KTextEditor::Document* doc, KTextEditor::Range keyRange, KTextEditor::Range valueRange,
                                             const SupportedProperty& property, const QString& value)
    : QWidget()
    , view(new QQuickWidget)
    , document(doc)
    , keyRange(keyRange)
    , valueRange(valueRange)
    , property(property)
{
    //setup kdeclarative library
    KDeclarative::KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(view->engine());
    kdeclarative.initialize();
    kdeclarative.setupBindings();        //binds things like kconfig and icons

    // Configure layout
    QHBoxLayout*l = new QHBoxLayout;

    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);

    // see docstring for ILanguageSupport::specialLanguageObjectNavigationWidget
    setProperty("DoNotCloseOnCursorMove", true);

    view->setSource(property.qmlfile);
    if (!view->rootObject()) {
        // don't crash because of a syntax error or missing QML file
        l->addWidget(new QLabel(i18n("Error loading QML file: %1", property.qmlfile.path())));
        delete view;
        return;
    }

    // set the initial value read from the document
    view->rootObject()->setProperty("value", value);

    // connect to the slot which has to be emitted from QML when the value changes
    QObject::connect(view->rootObject(), SIGNAL(valueChanged(QString)),
                     this, SLOT(updateValue(QString)));
    l->addWidget(view);
}
