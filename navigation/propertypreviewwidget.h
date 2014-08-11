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

#ifndef PROPERTYPREVIEWWIDGET_H
#define PROPERTYPREVIEWWIDGET_H

#include <QWidget>
#include <QFontDatabase>

#include <ktexteditor/range.h>
#include <language/duchain/declaration.h>
#include <interfaces/idocument.h>

using namespace KDevelop;
class QQuickWidget;

// Describes one supported property, such as "width"
struct SupportedProperty {
    SupportedProperty(const QUrl& qmlfile,
                      const QString &typeContains = QString(),
                      const QString &classContains = QString())
    : qmlfile(qmlfile),
      typeContains(typeContains),
      classContains(classContains)
    {
    }

    // the absolute (!) URL to the qml file to load when creating a widget
    // for this property
    QUrl qmlfile;
    // A string that must be contained into the string representation of the
    // type of the key being matched.
    QString typeContains;
    // A string that must be contained into the name of the class in which the
    // key is declared
    QString classContains;
};

// This class is responsible for creating the property widgets for editing QML properties
// with e.g. sliders. It knows which properties are supported, and creates a widget from
// a QML file for each supported property when requested. For the actual implementations
// of the widgets, see the propertywidgets/ subfolder, especially the README file.
class PropertyPreviewWidget : public QWidget {
Q_OBJECT
public:
    // Constructs a widget operating on the given document if the given key is in the list
    // of supported properties.
    // key and value must be the key and the current value of the property in question,
    // without spaces or other extra characters.
    // The ranges must encompass the text which should be replaced by the new values
    // selected by the user.
    // Returns 0 when the property is not supported, which tells kdevplatform not to
    // display any widget when returned from e.g. specialLanguageObjectNavigationWidget.
    static QWidget* constructIfPossible(KTextEditor::Document* doc,
                                        KTextEditor::Range keyRange,
                                        KTextEditor::Range valueRange,
                                        Declaration* decl,
                                        const QString& key,
                                        const QString& value);
    virtual ~PropertyPreviewWidget();

private:
    // private because you should use the static constructIfPossible function to create instances,
    // to make sure you don't have widgets which operate on unsupported properties.
    explicit PropertyPreviewWidget(KTextEditor::Document* doc, KTextEditor::Range keyRange, KTextEditor::Range valueRange,
                                   const SupportedProperty& property, const QString& value);
    static QHash<QString, SupportedProperty> supportedProperties;
    static QFontDatabase fontDatabase;

    QQuickWidget* view;

    // the document the widget replaces text in
    KTextEditor::Document* document;
    // the range of the key
    KTextEditor::Range const keyRange;
    // the range of the value to be modified. Not const because the range might change
    // if the newly inserted text is smaller or larger than what was there before
    // (e.g. 9 -> 10)
    KTextEditor::Range valueRange;
    // the SupportedProperty instance for this widget
    SupportedProperty const property;

private slots:
    // updates the text in the document to contain the new value in valueRange
    void updateValue();
};

#endif
