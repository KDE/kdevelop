#ifndef COLORCHOOSER_H
#define COLORCHOOSER_H

#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QLayout>
#include <QPainter>
#include <QMouseEvent>

#include <KTextEditor/Document>
#include <language/editor/simplerange.h>

#include <KColorValueSelector>
#include <KHueSaturationSelector>

class ColorChooser : public QWidget {
Q_OBJECT
public:
    ColorChooser(const QColor& startColor, KTextEditor::Document* document, const KDevelop::SimpleRange& range,
                 QWidget* parent = 0);
    QColor getSelectedColor();

private slots:
    void updateText();
    void updatePreview();
    void hueValueChanged(int hue, int saturation);
    void valueChanged(int value);

private:
    virtual bool event(QEvent* event);
    KTextEditor::Document* const document;
    KDevelop::SimpleRange range;
    int size;
    KColorValueSelector* valueSelector;
    KHueSaturationSelector* hueSaturationSelector;
    bool wasModified;
    QPixmap selectedColorPixmap;
    QLabel* selectedColorLabel;
};

#endif