#include "propertypreviewwidget.h"

#include <QDeclarativeView>
#include <QLayout>
#include <qgraphicsitem.h>
#include <KStandardDirs>
#include <KTextEditor/Document>
#include <KTextEditor/View>

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
    view->rootObject()->setProperty("value", newValue);
    document->activeView()->setCursorPosition(KTextEditor::Cursor(valueRange.start.line, valueRange.start.column));
    if ( valueRange.end.column - valueRange.start.column == newValue.size() ) {
        document->replaceText(valueRange.textRange(), newValue);
    }
    else {
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
    , slider(0)
    , wasChanged(false)
{
    setProperty("DoNotCloseOnCursorMove", true);
    view->setSource(property.qmlfile);
    view->rootObject()->setProperty("value", value);
    QObject::connect(view->rootObject(), SIGNAL(valueChanged(QString)),
                     this, SLOT(updateValue(QString)));
    setLayout(new QHBoxLayout);
    layout()->addWidget(view);
}
