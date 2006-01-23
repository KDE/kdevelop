/***************************************************************************
 *   Copyright (C) 2004-2005 by Alexander Dymo                             *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#include "buttonbar.h"

#include <QList>
#include <QResizeEvent>
#include <QBoxLayout>

#include <kdebug.h>
#include <kstringhandler.h>

#include "button.h"

namespace Ideal {

//ButtonLayout class

ButtonLayout::ButtonLayout(ButtonBar *parent, Direction d, int margin, int spacing, const char *name)
    :QBoxLayout(parent, d, margin, spacing, name), m_buttonBar(parent)
{
}

QSize ButtonLayout::minimumSize() const
{
    QSize size = QBoxLayout::minimumSize();

    if (!m_buttonBar->autoResize())
        return size;

    switch (m_buttonBar->place())
    {
        case Ideal::Left:
        case Ideal::Right:
            return QSize(size.width(),0);
            break;
        case Ideal::Top:
        case Ideal::Bottom:
            return QSize(0,size.height());
    }
    return QBoxLayout::minimumSize();
}



//ButtonBar class


ButtonBar::ButtonBar(Place place, ButtonMode mode, QWidget *parent, const char *name)
    :QWidget(parent, name), m_place(place), l(0), m_shrinked(false), m_autoResize(true)
{
    switch (m_place)
    {
        case Ideal::Left:
        case Ideal::Right:
           l = new ButtonLayout(this, QBoxLayout::TopToBottom, 0, 0);
            break;
        case Ideal::Top:
        case Ideal::Bottom:
            l = new ButtonLayout(this, QBoxLayout::LeftToRight, 0, 0);
            break;
    }

    l->setResizeMode(QLayout::SetMinimumSize);
    setMode(mode);

    l->insertStretch(-1);
}

ButtonBar::~ButtonBar()
{
}

void ButtonBar::addButton(Button *button)
{
    int buttonCount = m_buttons.count();

    button->setMode(m_mode);
    m_buttons.append(button);
    l->insertWidget(buttonCount, button);
    button->show();
    fixDimensions();
}

void ButtonBar::removeButton(Button *button)
{
    m_buttons.remove(button);
    l->remove(button);
    delete button;
}

void ButtonBar::setMode(ButtonMode mode)
{
    m_mode = mode;
    for (ButtonList::iterator it = m_buttons.begin(); it != m_buttons.end(); ++it)
        (*it)->setMode(mode);
}

ButtonMode ButtonBar::mode() const
{
    return m_mode;
}

Place ButtonBar::place() const
{
    return m_place;
}

void ButtonBar::fixDimensions()
{
    switch (m_place)
    {
        case Ideal::Left:
        case Ideal::Right:
            setFixedWidth(sizeHint().width());
            setMinimumHeight(sizeHint().height());
            setMaximumHeight(32767);
            break;
        case Ideal::Top:
        case Ideal::Bottom:
            setFixedHeight(sizeHint().height());
            setMinimumWidth(sizeHint().width());
            setMaximumWidth(32767);
            break;
    }
}

void ButtonBar::setButtonsPlace(Ideal::Place place)
{
    for (ButtonList::iterator it = m_buttons.begin(); it != m_buttons.end(); ++it)
        (*it)->setPlace(place);
}

void ButtonBar::resizeEvent(QResizeEvent *ev)
{
    int preferredDimension = 0;
    int actualDimension = 0;
    int oldDimension = 0;
    switch (m_place)
    {
        case Ideal::Left:
        case Ideal::Right:
            preferredDimension = l->QBoxLayout::minimumSize().height();
            actualDimension = size().height();
            oldDimension = ev->oldSize().height();
            break;
        case Ideal::Top:
        case Ideal::Bottom:
            preferredDimension = l->QBoxLayout::minimumSize().width();
            actualDimension = size().width();
            oldDimension = ev->oldSize().width();
            break;
    }

    if (preferredDimension > actualDimension)
        shrink(preferredDimension, actualDimension);
    else if (m_shrinked && (originalDimension() < actualDimension))
        unshrink();
    else if (m_shrinked && actualDimension > oldDimension)
        deshrink(preferredDimension, actualDimension);

    QWidget::resizeEvent(ev);
}

void ButtonBar::shrink(int preferredDimension, int actualDimension)
{
    if (!preferredDimension)
        return;

    m_shrinked = true;

    uint textLength = 0;
    QList<uint> texts;
    uint maxLength = 0;
    foreach (Button *button, m_buttons)
    {
        uint length = button->text().length();
        maxLength = length > maxLength ? length : maxLength ;
        texts.append(length);
        textLength += length;
    }

    uint newPreferredLength = actualDimension * textLength / preferredDimension;

    uint newMaxLength = maxLength;
    uint newTextLength;
    do {
        newMaxLength -= 1;
        newTextLength = 0;
        for (int i = 0; i < texts.count(); ++i)
        {
            uint &length = texts[i];
            if (length > newMaxLength)
                length = newMaxLength;
            newTextLength += length;
        }
    } while (newTextLength > newPreferredLength);

    int i = 0;
    foreach (Button *button, m_buttons)
    {
        button->setText(KStringHandler::rsqueeze(button->realText(), texts[i++]));
        button->updateSize();
    }
}

void ButtonBar::deshrink(int preferredDimension, int actualDimension)
{
    if (!preferredDimension)
        return;

    m_shrinked = true;

    uint textLength = 0;
    QList<uint> texts;
    uint maxLength = 0;
    foreach (Button *button, m_buttons)
    {
        uint length = button->text().length();
        maxLength = length > maxLength ? length : maxLength ;
        texts.append(length);
        textLength += length;
    }

    uint newPreferredLength = actualDimension * textLength / preferredDimension;

    if (newPreferredLength <= textLength)
        return;

    uint newTextLength;
    uint prevTextLength = 0;
    do {
        newTextLength = 0;
        for (int i = 0; i < texts.count(); ++i)
        {
            uint &length = texts[i];
            if (m_buttons.at(i)->text().contains("..."))
                length++;
            newTextLength += length;
        }
        if (newTextLength == prevTextLength)
            break;
        prevTextLength = newTextLength;
    } while (newTextLength < newPreferredLength);

    int i = 0;
    foreach (Button *button, m_buttons)
    {
        if (texts[i] >= button->realText().length())
            button->setText(button->realText());
        else
            button->setText(KStringHandler::rsqueeze(button->realText(), texts[i]));
        button->updateSize();
        ++i;
    }
}

void ButtonBar::unshrink()
{
    foreach (Button *button, m_buttons)
    {
        button->setText(button->realText());
        button->updateSize();
    }
    m_shrinked = false;
}

int ButtonBar::originalDimension()
{
    int size = 0;
    foreach (Button *button, m_buttons)
    {
        size += button->sizeHint(button->realText()).width();
    }
    return size;
}

bool ButtonBar::autoResize() const
{
    return m_autoResize;
}

void ButtonBar::setAutoResize(bool b)
{
    m_autoResize = b;
}

}

#include "buttonbar.moc"
