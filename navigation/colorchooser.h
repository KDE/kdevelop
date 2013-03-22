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

// This class is a simple color chooser based on the KDE color hue/saturation
// and value selectors. It is used for the popup widget which appears when the mouse
// is moved over a color identifier.
class ColorChooser : public QWidget {
Q_OBJECT
public:
    ColorChooser(const QColor& startColor, KTextEditor::Document* document, const KDevelop::SimpleRange& range,
                 QWidget* parent = 0);
    // returns the currently selected color
    QColor getSelectedColor();

private slots:
    // update the text in the document
    void updateText();
    // refill the preview pixmap with the currently selected color
    void updatePreview();
    // update hue and value of both widgets when the user changes the hue/value
    void hueValueChanged(int hue, int saturation);
    // update color value on both widgets when the user changes the value
    void valueChanged(int value);

private:
    virtual bool event(QEvent* event);
    // the document replacements should be done in
    KTextEditor::Document* const document;
    // the range of the text which should be replaced
    KDevelop::SimpleRange range;
    // the width/height of the xy selector, and the height of the y selector
    const int size;
    // the slider for the color value
    KColorValueSelector* valueSelector;
    // the xy selector for the color hue / saturation
    KHueSaturationSelector* hueSaturationSelector;
    // true if the color was changed by the user in the widget sice it has been opened
    bool wasModified;
    // pixmap filled with the preview color
    // TODO could we do this without a pixmap?
    QPixmap selectedColorPixmap;
    // label for the selected color, displays the pixmap
    QLabel* selectedColorLabel;
};

#endif