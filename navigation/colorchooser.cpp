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
    // ChooserValue is is the default mode which is used in kcolorchooser,
    // so use it here too
    hueSaturationSelector->setChooserMode(ChooserValue);
    selectedColorLabel->setFixedSize(size/2, size/4);
    setFocusPolicy(Qt::NoFocus);
    // see docstring for ILanguageSupport::specialLanguageObjectNavigationWidget
    setProperty("DoNotCloseOnCursorMove", true);
    setLayout(new QHBoxLayout);

    hueSaturationSelector->setFixedSize(size, size);
    // both widgets are given all color values, because their display style
    // also depends on the color components they are not responsible to select
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
    // whenever any value changes,
    //  * update the preview color
    //  * synchronize the changes with the other widget
    //  * update the color in the text document
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
        // all modifications are done in *one* editing step, because
        // the user can then undo selecting a different color in the widget
        // by using "Undo" once. Without this, you'd need one "undo" operation
        // per pixel your mouse moved, which is annoying.
        // TODO: which kind of bad things can happen when the document
        // is in editing mode for several seconds?
        // One thing that happens for sure is that if the user types stuff while
        // the widget is open, highlighting doesn't update.
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
        // see above
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
    // could translate back to readable name,  but it's not worth it
    // (would need to be done manually with a lookup table)
    const QString name = newColor.toRgb().name();
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
        // when the widget is hidden, update the text for one last time, then commit and close.
        updateText();
        document->endEditing();
    }
    return QWidget::event(event);
};
