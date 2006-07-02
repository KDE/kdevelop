/***************************************************************************
 *   Copyright (C) 2004-2006 by Alexander Dymo                             *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#include "buttoncontainer.h"

#include <QResizeEvent>

#include "button.h"

#include <QDebug>

namespace Ideal {

//ButtonLayout class

ButtonLayout::ButtonLayout(Direction dir, ButtonContainer *parent)
    :QBoxLayout(dir, parent), m_buttonBar(parent)
{
    setMargin(0);
    setSpacing(0);
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



//ButtonContainer class


ButtonContainer::ButtonContainer(Place place, ButtonMode mode, QWidget *parent)
    :QWidget(parent), m_place(place), m_buttonLayout(0),
    m_shrinked(false), m_autoResize(true)
{
    switch (m_place)
    {
        case Ideal::Left:
        case Ideal::Right:
            m_buttonLayout = new ButtonLayout(QBoxLayout::TopToBottom, this);
            break;
        case Ideal::Top:
        case Ideal::Bottom:
            m_buttonLayout = new ButtonLayout(QBoxLayout::TopToBottom, this);
            break;
    }

    m_buttonLayout->setSizeConstraint(QLayout::SetMinimumSize);
    setMode(mode);

    m_buttonLayout->insertStretch(-1);
}

ButtonContainer::~ButtonContainer()
{
}

void ButtonContainer::addButton(Button *button, bool isShown)
{
    int buttonCount = m_buttons.count();

    button->setMode(m_mode);
    m_buttons.append(button);
    m_buttonLayout->insertWidget(buttonCount, button);
    if (isShown)
        button->show();
    fixDimensions();
}

void ButtonContainer::removeButton(Button *button)
{
    m_buttons.removeAll(button);
    m_buttonLayout->removeWidget(button);
    delete button;
}

void ButtonContainer::setMode(ButtonMode mode)
{
    m_mode = mode;
    foreach (Button *button, m_buttons)
        button->setMode(mode);
}

ButtonMode ButtonContainer::mode() const
{
    return m_mode;
}

Place ButtonContainer::place() const
{
    return m_place;
}

void ButtonContainer::fixDimensions()
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

void ButtonContainer::setButtonsPlace(Ideal::Place place)
{
    foreach (Button *button, m_buttons)
        button->setPlace(place);
}

void ButtonContainer::resizeEvent(QResizeEvent *ev)
{
    int preferredDimension = 0;
    int actualDimension = 0;
    int oldDimension = 0;
    switch (m_place)
    {
        case Ideal::Left:
        case Ideal::Right:
            preferredDimension = m_buttonLayout->QBoxLayout::minimumSize().height();
            actualDimension = size().height();
            oldDimension = ev->oldSize().height();
            break;
        case Ideal::Top:
        case Ideal::Bottom:
            preferredDimension = m_buttonLayout->QBoxLayout::minimumSize().width();
            actualDimension = size().width();
            oldDimension = ev->oldSize().width();
            break;
    }

    qDebug() << "pref: " << preferredDimension << " act: " << actualDimension;

    if (preferredDimension > actualDimension)
        shrink(preferredDimension, actualDimension);
    else if (m_shrinked && (originalDimension() < actualDimension))
        unshrink();
    else if (m_shrinked && actualDimension > oldDimension)
        deshrink(preferredDimension, actualDimension);

    QWidget::resizeEvent(ev);
}

void ButtonContainer::shrink(int preferredDimension, int actualDimension)
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
        for (QList<uint>::iterator it = texts.begin(); it != texts.end(); ++it)
        {
            if (*it > newMaxLength)
                *it = newMaxLength;
            newTextLength += *it;
        }
    } while (newTextLength > newPreferredLength);

    int i = 0;
    foreach (Button *button, m_buttons)
    {
        button->setText(squeeze(button->realText(), texts[i++]));
        button->updateSize();
    }
}

void ButtonContainer::deshrink(int preferredDimension, int actualDimension)
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

    qDebug() << newPreferredLength << " " << textLength;

    uint newTextLength;
    uint prevTextLength = 0;
    do {
        newTextLength = 0;
        int i = 0;
        for (QList<uint>::iterator it = texts.begin(); it != texts.end(); ++it, i++)
        {
            if (m_buttons[i]->text().contains("..."))
                (*it)++;
            newTextLength += *it;
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
            button->setText(squeeze(button->realText(), texts[i]));
        button->updateSize();
        ++i;
    }
}

void ButtonContainer::unshrink()
{
    foreach (Button *button, m_buttons)
    {
        button->setText(button->realText());
        button->updateSize();
    }
    m_shrinked = false;
}

int ButtonContainer::originalDimension()
{
    int size = 0;
    foreach (Button *button, m_buttons)
    {
        size += button->sizeHint(button->realText()).width();
    }
    return size;
}

bool ButtonContainer::autoResize() const
{
    return m_autoResize;
}

void ButtonContainer::setAutoResize(bool b)
{
    m_autoResize = b;
}

QString ButtonContainer::squeeze(const QString &str, int maxlen)
{
    if (str.length() > maxlen)
    {
        int part = maxlen-3;
        return QString(str.left(part) + "...");
    }
    else
        return str;
}

bool ButtonContainer::isEmpty()
{
    return !m_buttons.count();
}

}

#include "buttoncontainer.moc"
