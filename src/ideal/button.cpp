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
#include "button.h"

#include <QPainter>
#include <QToolTip>
#include <QStyle>
#include <QStyleOption>
#include <QStylePainter>
#include <QApplication>
#include <QPixmap>

#include "buttoncontainer.h"

namespace Ideal {

struct ButtonPrivate {
    ButtonContainer *buttonBar;

    QString description;
    Place place;

    QString realText;
    QIcon realIcon;
};

Button::Button(ButtonContainer *parent, const QString text, const QIcon &icon,
    const QString &description)
    :QPushButton(icon, text, parent)
{
    d = new ButtonPrivate;
    d->buttonBar = parent;
    d->description = description;
    d->place = parent->place();
    d->realText = text;
    d->realIcon = icon;

    hide();
    setFlat(true);
    setCheckable(true);
    setFocusPolicy(Qt::NoFocus);
    setDescription(d->description);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    resize(sizeHint());
    fixDimensions(Ideal::Bottom);

    setToolTip(d->realText);
}

Button::~Button()
{
//     d->buttonBar->removeButton(this);
}

void Button::setDescription(const QString &description)
{
    d->description = description;
    setToolTip(d->description);
}

QString Button::description() const
{
    return d->description;
}

QStyleOptionButton Button::styleOption() const
{
    QStyleOptionButton opt;
    opt.init(this);

    if (isEnabled())
        opt.state |= QStyle::State_Enabled;
    if (hasFocus())
        opt.state |= QStyle::State_HasFocus;
    if (isDown())
        opt.state |= QStyle::State_Sunken;
    if (isChecked())
        opt.state |= QStyle::State_On;
    if (! isFlat() && ! isDown())
        opt.state |= QStyle::State_Raised;
    opt.features = isDefault() ? QStyleOptionButton::DefaultButton : QStyleOptionButton::None;

    opt.text = text();

    opt.icon = d->realIcon;
    opt.iconSize = QSize(16,16);

    QRect r = rect();
    if (d->place == Ideal::Left || d->place == Ideal::Right)
        r.setSize(QSize(r.height(), r.width()));

    opt.rect = r;

    return opt;
}

void Button::paintEvent(QPaintEvent *)
{
    QStyleOptionButton opt = styleOption();
    QRect r = opt.rect;

    QPixmap pm(r.width(), r.height());
//     pm.fill(QColor(255,255,255,0));

    QStylePainter p2(&pm, this);
    p2.drawControl(QStyle::CE_PushButton, opt);

    QPainter p(this);

    switch (d->place)
    {
        case Ideal::Left:
                p.rotate(-90);
                p.drawPixmap(1-pm.width(), 0, pm);
                break;
        case Ideal::Right:
                p.rotate(90);
                p.drawPixmap(0, 1-pm.height(), pm);
                break;
        default:
                p.drawPixmap(0, 0, pm);
                break;
    }
}

ButtonMode Button::mode()
{
    return d->buttonBar->mode();
}

void Button::setPlace(Ideal::Place place)
{
    Place oldPlace = d->place;
    d->place = place;
    fixDimensions(oldPlace);
}

void Button::fixDimensions(Place oldPlace)
{
    switch (d->place)
    {
        case Ideal::Left:
        case Ideal::Right:
            if ((oldPlace == Ideal::Bottom) || (oldPlace == Ideal::Top))
            {
                setFixedWidth(height());
                setMinimumHeight(sizeHint().width());
                setMaximumHeight(32767);
            }
            break;
        case Ideal::Top:
        case Ideal::Bottom:
            if ((oldPlace == Ideal::Left) || (oldPlace == Ideal::Right))
            {
                setFixedHeight(width());
                setMinimumWidth(sizeHint().height());
                setMaximumWidth(32767);
            }
            break;
    }
}

QSize Button::sizeHint() const
{
    return sizeHint(text());
}

QSize Button::sizeHint(const QString &text) const
{
    ensurePolished();
    QStyleOptionButton option = styleOption();
    int w = 0, h = 0;

    if ( !icon().isNull() && (d->buttonBar->mode() != Text) ) {
        int iw = iconSize().width();
        int ih = iconSize().height();
        w += iw;
        h = qMax( h, ih );
    }
    if ( menu() ) {
        w += style()->pixelMetric(QStyle::PM_MenuButtonIndicator, &option, this);
    }
    QString s( text );
    bool empty = s.isEmpty();
    if ( empty )
        s = QLatin1String("XXXX");
    QFontMetrics fm = fontMetrics();
    QSize sz = fm.size( Qt::TextShowMnemonic, s );
    if(!empty || !w)
        w += sz.width();
    if(!empty || !h)
        h = qMax(h, sz.height());

    return (style()->sizeFromContents(QStyle::CT_ToolButton, &option, QSize(w, h), this).
            expandedTo(QApplication::globalStrut()));
}

void Ideal::Button::updateSize()
{
    switch (d->place)
    {
        case Ideal::Left:
        case Ideal::Right:
             setMinimumHeight(sizeHint().width());
             resize(sizeHint().height(), sizeHint().width());
            break;
        case Ideal::Top:
        case Ideal::Bottom:
            resize(sizeHint().width(), sizeHint().height());
            break;
    }
}

QString Button::realText() const
{
    return d->realText;
}

void Button::setMode(Ideal::ButtonMode mode)
{
    switch (mode)
    {
        case Text:
            disableIcon();
            enableText();
            break;
        case IconsAndText:
            enableIcon();
            enableText();
            break;
        case Icons:
            disableText();
            enableIcon();
            break;
    }
}

void Button::enableIcon()
{
    if (!icon().isNull())
    {
        if (d->realIcon.isNull())
        {
            QPixmap empty(16, 16);
            empty.fill(QColor(255,255,255,0));
            d->realIcon = QIcon(empty);
        }
        setIcon(d->realIcon);
    }
}

void Button::disableIcon()
{
    setIcon(QIcon());
}

void Button::disableText()
{
    if (text().length() > 0)
        setText("");
}

void Button::enableText()
{
    setText(d->realText);
}

}

#include "button.moc"
