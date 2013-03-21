#include "colorchooser.h"
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iplugincontroller.h>
#include <KTextEditor/View>

#include <KColorValueSelector>
#include <KHueSaturationSelector>

ColorChooser::ColorChooser(const QColor& startColor, KTextEditor::Document* document, const KDevelop::SimpleRange& range, QWidget* parent)
    : QWidget(parent)
    , document(document)
    , range(range)
    , size(70)
    , valueSelector(new KColorValueSelector)
    , hueSaturationSelector(new KHueSaturationSelector)
    , wasModified(false)
    , selectedColorLabel(new QLabel)
    , selectedColorPixmap(size, size)
{
    hueSaturationSelector->setChooserMode(ChooserValue);
    selectedColorLabel->setFixedSize(size/2, size/4);
    setFocusPolicy(Qt::NoFocus);
    setProperty("DoNotCloseOnCursorMove", true);
    setLayout(new QHBoxLayout);

    hueSaturationSelector->setFixedSize(size, size);
    hueSaturationSelector->setHue(startColor.hsvHue());
    hueSaturationSelector->setSaturation(startColor.hsvSaturation());
    hueSaturationSelector->setColorValue(startColor.value());
    valueSelector->setFixedHeight(size + 8);
    valueSelector->setFixedWidth(20);
    valueSelector->setColorValue(startColor.value());
    valueSelector->setHue(startColor.hsvHue());
    valueSelector->setSaturation(startColor.hsvSaturation());
    valueSelector->setValue(startColor.value());

    layout()->addWidget(hueSaturationSelector);
    layout()->addWidget(valueSelector);
    layout()->addWidget(selectedColorLabel);
    connect(hueSaturationSelector, SIGNAL(valueChanged(int,int)), this, SLOT(updatePreview()));
    connect(valueSelector, SIGNAL(valueChanged(int)), this, SLOT(updatePreview()));
    connect(hueSaturationSelector, SIGNAL(valueChanged(int,int)), this, SLOT(hueValueChanged(int,int)));
    connect(valueSelector, SIGNAL(valueChanged(int)), this, SLOT(valueChanged(int)));
    updatePreview();
    connect(hueSaturationSelector, SIGNAL(valueChanged(int,int)), this, SLOT(updateText()));
    connect(valueSelector, SIGNAL(valueChanged(int)), this, SLOT(updateText()));
};

void ColorChooser::hueValueChanged(int hue, int saturation)
{
    if ( ! wasModified ) {
        document->startEditing();
        wasModified = true;
    }
    hueSaturationSelector->setHue(hue);
    hueSaturationSelector->setSaturation(saturation);
    valueSelector->setHue(hue);
    valueSelector->setSaturation(saturation);
}

void ColorChooser::valueChanged(int value)
{
    if ( ! wasModified ) {
        document->startEditing();
        wasModified = true;
    }
    valueSelector->setColorValue(value);
    hueSaturationSelector->setColorValue(value);
}

void ColorChooser::updatePreview()
{
    hueSaturationSelector->updateContents();
    valueSelector->updateContents();
    hueSaturationSelector->repaint();
    valueSelector->repaint();
    selectedColorPixmap.fill(getSelectedColor());
    selectedColorLabel->setPixmap(selectedColorPixmap);
}

QColor ColorChooser::getSelectedColor()
{
    return QColor::fromHsv(hueSaturationSelector->hue(), hueSaturationSelector->saturation(), valueSelector->colorValue());
}

void ColorChooser::updateText()
{
    // don't change anything if the user did not modify the color.
    // this prevents mesing up code in case something goes wrong
    if ( ! wasModified ) {
        return;
    }
    document->activeView()->setCursorPosition(range.textRange().start());
    QColor newColor = getSelectedColor();
    // could translate back to readable name,  but it's not worth it.
    const QString name = newColor.toRgb().name();
    qDebug() << range.end.column - range.start.column << name.size();
    if ( range.end.column - range.start.column == name.size() ) {
        document->replaceText(range.textRange(), name);
    }
    else {
        document->removeText(range.textRange());
        document->insertText(range.textRange().start(), name);
        range.end.column = range.start.column + name.size();
        // workaround to update the highlighting -- TODO does it make sense usability-wise?
        document->endEditing();
        document->startEditing();
    }
};

bool ColorChooser::event(QEvent* event)
{
    if ( event->type() == QEvent::Hide && wasModified ) {
        updateText();
        document->endEditing();
    }
    return QWidget::event(event);
};
