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
#warning fix: kstandarddirs
        KStandardDirs d;
        supportedProperties["width"] = SupportedProperty(
            QUrl("/home/sven/Projekte/kde/kdev-qmljs/navigation/propertywidgets/width.qml"),
            SupportedProperty::IntegerValues, QPointF(0, 25), QStringList(),
            SupportedProperty::Squared, SupportedProperty::Horizontal
        );
    }
    QHash<QString, SupportedProperty>::iterator item = supportedProperties.find(key);
    if ( item != supportedProperties.end() ) {
        return new PropertyPreviewWidget(doc, keyRange, valueRange, *item, value);
    }
    return 0;
}

void PropertyPreviewWidget::updateValue(const QString& newValue)
{
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
    document->endEditing();
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
{
    setProperty("DoNotCloseOnCursorMove", true);
    view->setSource(property.qmlfile);
    setFixedSize(220, 100);
    view->rootObject()->setProperty("width", 220);
    view->rootObject()->setProperty("height", 100);
    view->rootObject()->setProperty("initialValue", value);
    QObject::connect(view->rootObject(), SIGNAL(valueChanged(QString)),
                     this, SLOT(updateValue(QString)));
    setLayout(new QHBoxLayout);
    layout()->addWidget(view);
    doc->startEditing();
}
