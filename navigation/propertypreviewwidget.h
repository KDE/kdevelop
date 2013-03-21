#ifndef PROPERTYPREVIEWWIDGET_H
#define PROPERTYPREVIEWWIDGET_H
#include <QWidget>
#include <QSlider>
#include <QDeclarativeView>

#include <language/editor/documentrange.h>
#include <interfaces/idocument.h>

using namespace KDevelop;

struct SupportedProperty {
    enum PossibleValues {
        IntegerValues,
        FloatingPointValues,
        FixedStringValues
    };
    enum Easing {
        Linear,
        Squared
    };
    enum OrientationHint {
        Horizontal,
        Vertical
    };
    SupportedProperty() { };
    SupportedProperty(const QUrl& qmlfile, PossibleValues values = IntegerValues, const QPointF& dataRange = QPointF(0, 100),
                      const QStringList& possibleFixedValues = QStringList(), Easing easing = Linear,
                      OrientationHint orientation = Horizontal)
        : qmlfile(qmlfile)
        , possibleFixedValues(possibleFixedValues)
        , dataRange(dataRange)
        , possibleValues(values)
        , easing(easing)
        , orientationHint(orientation)
    { };

    QUrl qmlfile;

    QStringList possibleFixedValues;
    QPointF dataRange;
    PossibleValues possibleValues;
    Easing easing;
    OrientationHint orientationHint;
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

private slots:
    void updateValue(const QString&);
};

#endif