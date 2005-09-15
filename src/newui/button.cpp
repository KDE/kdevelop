/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "button.h"

#include <qpainter.h>
#include <qtooltip.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qstylepainter.h>
#include <qapplication.h>
#include <QPixmap>

#include <kdebug.h>
#include <kiconloader.h>

#include "buttonbar.h"

namespace Ideal {

Button::Button(ButtonBar *parent, const QString text, const QIcon &icon,
    const QString &description)
    :QPushButton(icon, text, parent), m_buttonBar(parent), m_description(description),
    m_place(parent->place()), m_realText(text), m_realIconSet(icon)
{
    hide();
    setFlat(true);
    setToggleButton(true);
    setFocusPolicy(Qt::NoFocus);
    setDescription(m_description);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    resize(sizeHint());
    fixDimensions(Ideal::Bottom);

    QToolTip::add(this, m_realText);
}

Button::~Button()
{
//     m_buttonBar->removeButton(this);
}

void Button::setDescription(const QString &description)
{
    m_description = description;
    QToolTip::remove(this);
    QToolTip::add(this, m_description);
}

QString Button::description() const
{
    return m_description;
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
    if (isOn())
        opt.state |= QStyle::State_On;
    if (! isFlat() && ! isDown())
        opt.state |= QStyle::State_Raised;
    opt.features = isDefault() ? QStyleOptionButton::DefaultButton : QStyleOptionButton::None;

    opt.text = text();

    opt.icon = m_realIconSet;
    opt.iconSize = QSize(16,16);

    QRect r = rect();
    if (m_place == Ideal::Left || m_place == Ideal::Right)
        r.setSize(QSize(r.height(), r.width()));

    opt.rect = r;

    return opt;
}

void Button::paintEvent(QPaintEvent *)
{
    QStyleOptionButton opt = styleOption();
    QRect r = opt.rect;

    QPixmap pm(r.width(), r.height());
    pm.fill(eraseColor());

    QStylePainter p2(&pm, this);
    p2.drawControl(QStyle::CE_PushButton, opt);

    QPainter p(this);

    switch (m_place)
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
    return m_buttonBar->mode();
}

void Button::setPlace(Ideal::Place place)
{
    Place oldPlace = m_place;
    m_place = place;
    fixDimensions(oldPlace);
}

void Button::fixDimensions(Place oldPlace)
{
    switch (m_place)
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
    constPolish();
    QStyleOptionButton option = styleOption();
    int w = 0, h = 0;

    if ( iconSet() && !iconSet()->isNull() && (m_buttonBar->mode() != Text) ) {
        int iw = iconSet()->pixmap( QIcon::Small, QIcon::Normal ).width() + 4;
        int ih = iconSet()->pixmap( QIcon::Small, QIcon::Normal ).height();
        w += iw;
        h = QMAX( h, ih );
    }
    if ( isMenuButton() ) {
        w += style()->pixelMetric(QStyle::PM_MenuButtonIndicator, &option, this);
    }
    if ( pixmap() ) {
        QPixmap *pm = (QPixmap *)pixmap();
        w += pm->width();
        h += pm->height();
    } else if (m_buttonBar->mode() != Icons) {
        QString s( text );
        bool empty = s.isEmpty();
        if ( empty )
            s = QLatin1String("XXXX");
        QFontMetrics fm = fontMetrics();
        QSize sz = fm.size( Qt::TextShowMnemonic, s );
        if(!empty || !w)
            w += sz.width();
        if(!empty || !h)
            h = QMAX(h, sz.height());
    }

    return (style()->sizeFromContents(QStyle::CT_ToolButton, &option, QSize(w, h), this).
            expandedTo(QApplication::globalStrut()));
}

void Ideal::Button::updateSize()
{
    switch (m_place)
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
    return m_realText;
}

void Button::setMode(Ideal::ButtonMode mode)
{
    switch (mode)
    {
        case Text:
            disableIconSet();
            enableText();
            break;
        case IconsAndText:
            enableIconSet();
            enableText();
            break;
        case Icons:
            disableText();
            enableIconSet();
            break;
    }
}

void Button::enableIconSet()
{
    if (!iconSet())
    {
        if (m_realIconSet.isNull())
            m_realIconSet = SmallIcon("file_new");
        setIconSet(m_realIconSet);
    }
}

void Button::disableIconSet()
{
    setIconSet(QIcon());
}

void Button::disableText()
{
    if (text().length() > 0)
        setText("");
}

void Button::enableText()
{
    setText(m_realText);
}

}

#include "button.moc"
