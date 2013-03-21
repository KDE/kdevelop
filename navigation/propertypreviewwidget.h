#ifndef PROPERTYPREVIEWWIDGET_H
#define PROPERTYPREVIEWWIDGET_H
#include <QWidget>
#include <QSlider>
#include <QDeclarativeView>

#include <language/editor/documentrange.h>
#include <interfaces/idocument.h>

using namespace KDevelop;

struct SupportedProperty {
    SupportedProperty() { };
    SupportedProperty(const QUrl& qmlfile, const QStringList fixedValues = QStringList())
        : qmlfile(qmlfile)
        , possibleFixedValues(fixedValues)
    { };

    QUrl qmlfile;
    QStringList possibleFixedValues;
};

class PropertyPreviewWidget : public QWidget {
Q_OBJECT
public:
    static QWidget* constructIfPossible(KTextEditor::Document* doc, SimpleRange keyRange, SimpleRange valueRange,
                                        const QString& key, const QString& value);
    virtual ~PropertyPreviewWidget();

private:
    explicit PropertyPreviewWidget(KTextEditor::Document* doc, SimpleRange keyRange, SimpleRange valueRange,
                                   const SupportedProperty& property, const QString& value);
    static QHash<QString, SupportedProperty> supportedProperties;

    QDeclarativeView* view;
    QSlider* slider;

    KTextEditor::Document* document;
    SimpleRange const keyRange;
    SimpleRange valueRange;
    SupportedProperty const property;
    bool wasChanged;

private slots:
    void updateValue(const QString&);
};

#endif