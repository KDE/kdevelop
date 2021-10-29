/*
    SPDX-FileCopyrightText: 2013 Sven Brauch <svenbrauch@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PROPERTYPREVIEWWIDGET_H
#define PROPERTYPREVIEWWIDGET_H

#include <QWidget>
#include <QMultiHash>

#include <language/duchain/declaration.h>
#include <interfaces/idocument.h>

#include <KTextEditor/Range>

using namespace KDevelop;
class QQuickWidget;

// Describes one supported property, such as "width"
struct SupportedProperty {
    explicit SupportedProperty(const QUrl& qmlfile,
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

Q_DECLARE_TYPEINFO(SupportedProperty, Q_MOVABLE_TYPE);


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
                                        const KTextEditor::Range& keyRange,
                                        const KTextEditor::Range& valueRange,
                                        Declaration* decl,
                                        const QString& key,
                                        const QString& value);
    ~PropertyPreviewWidget() override;

private:
    // private because you should use the static constructIfPossible function to create instances,
    // to make sure you don't have widgets which operate on unsupported properties.
    explicit PropertyPreviewWidget(KTextEditor::Document* doc,
                                   const KTextEditor::Range& keyRange, const KTextEditor::Range& valueRange,
                                   const SupportedProperty& property, const QString& value);
    static QMultiHash<QString, SupportedProperty> supportedProperties;

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

private Q_SLOTS:
    // updates the text in the document to contain the new value in valueRange
    void updateValue();
};

#endif
