/* KDevelop xUnit plugin
 *
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "overlaytoggle.h"
#include "../test.h"

#include <QPainter>
#include <QPaintEvent>
#include <QRect>
#include <QTimer>
#include <QTimeLine>

#include <KDebug>
#include <KGlobalSettings>
#include <KIcon>
#include <KIconLoader>
#include <KIconEffect>
#include <KLocale>

using Veritas::SelectionToggle;
using Veritas::VerboseToggle;
using Veritas::ToSourceToggle;
using Veritas::OverlayButton;
using Veritas::Test;

QModelIndex OverlayButton::index()
{
    return m_index;
}

void OverlayButton::setIndex(const QModelIndex& index)
{
    m_index = index;
}

OverlayButton::~OverlayButton()
{
}

void OverlayButton::reset()
{
    m_index = QModelIndex();
    hide();
}

QSize OverlayButton::sizeHint() const
{
    return QSize(16, 16);
}


void OverlayButton::setVisible(bool visible)
{
    QAbstractButton::setVisible(visible);

    stopFading();
    if (visible) {
        startFading();
    }

}

OverlayButton::OverlayButton(QWidget* parent) : QAbstractButton(parent),
    m_icon(),
    m_isHovered(false),
    m_fadingValue(0),
    m_fadingTimeLine(0)
{}


void OverlayButton::setFadingValue(int value)
{
    m_fadingValue = value;
    if (m_fadingValue >= 255) {
        Q_ASSERT(m_fadingTimeLine != 0);
        m_fadingTimeLine->stop();
    }
    update();
}

void OverlayButton::startFading()
{
    Q_ASSERT(m_fadingTimeLine == 0);

    const bool animate = KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects;
    const int duration = animate ? 600 : 1;

    m_fadingTimeLine = new QTimeLine(duration, this);
    connect(m_fadingTimeLine, SIGNAL(frameChanged(int)),
            this, SLOT(setFadingValue(int)));
    m_fadingTimeLine->setFrameRange(0, 255);
    m_fadingTimeLine->start();
    m_fadingValue = 0;
}

void OverlayButton::stopFading()
{
    if (m_fadingTimeLine != 0) {
        m_fadingTimeLine->stop();
        delete m_fadingTimeLine;
        m_fadingTimeLine = 0;
    }
    m_fadingValue = 0;
}

void OverlayButton::leaveEvent(QEvent* event)
{
    QAbstractButton::leaveEvent(event);
    m_isHovered = false;
    update();
}

void OverlayButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    const QPalette& palette = parentWidget()->palette();

    const QBrush& backgroundBrush = palette.brush(QPalette::Normal, QPalette::Window);
    QColor background = backgroundBrush.color();
    background.setAlpha(m_fadingValue / 2);
    painter.setBrush(background);

    const QBrush& foregroundBrush = palette.brush(QPalette::Normal, QPalette::WindowText);
    QColor foreground = foregroundBrush.color();
    foreground.setAlpha(m_fadingValue / 4);
    painter.setPen(foreground);

    // draw the icon overlay
    if (m_isHovered) {
        KIconEffect iconEffect;
        QPixmap activeIcon = iconEffect.apply(m_icon, KIconLoader::Desktop, KIconLoader::ActiveState);
        painter.drawPixmap(0, 0, activeIcon);
    } else {
        if (m_fadingValue < 255) {
            // apply an alpha mask respecting the fading value to the icon
            QPixmap icon = m_icon;
            QPixmap alphaMask(icon.width(), icon.height());
            const QColor color(m_fadingValue, m_fadingValue, m_fadingValue);
            alphaMask.fill(color);
            icon.setAlphaChannel(alphaMask);
            painter.drawPixmap(0, 0, icon);
        } else {
            // no fading is required
            painter.drawPixmap(0, 0, m_icon);
        }
    }
}

void OverlayButton::enterEvent(QEvent* event)
{
    QAbstractButton::enterEvent(event);

    // if the mouse cursor is above the selection toggle, display
    // it immediately without fading timer
    m_isHovered = true;
    if (m_fadingTimeLine != 0) {
        m_fadingTimeLine->stop();
    }
    m_fadingValue = 255;
    update();
}


bool OverlayButton::eventFilter(QObject* obj, QEvent* event)
{
    if ((obj == parent()) && (event->type() == QEvent::Leave)) {
        hide();
    }
    return QAbstractButton::eventFilter(obj, event);
}

//////////////////////////// SelectionToggle /////////////////////////////////

bool SelectionToggle::shouldShow(Test* t)
{
    return t != 0 && t->needSelectionToggle();
}

SelectionToggle::SelectionToggle(QWidget* parent) :
    OverlayButton(parent)
{
    setFocusPolicy(Qt::NoFocus);
    parent->installEventFilter(this);
    resize(sizeHint());
    setIconOverlay(isChecked());
    connect(this, SIGNAL(toggled(bool)),
            this, SLOT(setIconOverlay(bool)));
    connect(KGlobalSettings::self(), SIGNAL(iconChanged(int)),
            this, SLOT(refreshIcon()));
}

SelectionToggle::~SelectionToggle()
{
}

void SelectionToggle::setIconOverlay(bool checked)
{
    const char* icon = 0;
    if (checked) icon = "list-remove";
    else         icon = "list-add";
    m_icon = KIconLoader::global()->loadIcon(icon,
                                             KIconLoader::NoGroup,
                                             KIconLoader::SizeSmall);
    setToolTip(checked ? i18nc("@info:tooltip", "Deselect Item") :
                             i18nc("@info:tooltip", "Select Item"));
    update();
}

void SelectionToggle::refreshIcon()
{
    setIconOverlay(isChecked());
}

////////// VerboseToggle /////////////////////////////////////////////////////


VerboseToggle::VerboseToggle(QWidget* parent) :
    OverlayButton(parent)
{
    setFocusPolicy(Qt::NoFocus);
    parent->installEventFilter(this);
    resize(sizeHint());
    setIconOverlay();
    connect(KGlobalSettings::self(), SIGNAL(iconChanged(int)),
            this, SLOT(setIconOverlay()));
    setToolTip(i18nc("@info:tooltip", "Verbose Output"));
}

VerboseToggle::~VerboseToggle()
{
}

bool VerboseToggle::shouldShow(Test* t)
{
    return t!=0 && t->needVerboseToggle();
}

void VerboseToggle::setIconOverlay()
{
    m_icon = KIconLoader::global()->loadIcon(
        "utilities-log-viewer",
        KIconLoader::NoGroup,
        KIconLoader::SizeSmall);
    update();
}

int VerboseToggle::offset(Test*)
{
    return 34;
}

//////////////// ToSourceToggle //////////////////////////////////////////////

bool ToSourceToggle::shouldShow(Test* t)
{
    return t != 0 && t->supportsToSource();
}

ToSourceToggle::ToSourceToggle(QWidget* parent) :
    OverlayButton(parent)
{
    setFocusPolicy(Qt::NoFocus);
    parent->installEventFilter(this);
    resize(sizeHint());
    setIcon();
    setToolTip(i18nc("@info:tooltip", "To Source"));
    connect(KGlobalSettings::self(), SIGNAL(iconChanged(int)),
            this, SLOT(refreshIcon()));
}

ToSourceToggle::~ToSourceToggle()
{
}

int ToSourceToggle::offset(Test* t)
{
    if (!t) return 0;
    int offset = 17;
    if (t->needSelectionToggle()) {
        offset += 17;
    }
    if (t->needVerboseToggle()) {
        offset += 17;
    }
    return offset;
}

void ToSourceToggle::setIcon()
{
    m_icon = KIconLoader::global()->loadIcon("text-x-c++src",
                                             KIconLoader::NoGroup,
                                             KIconLoader::SizeSmall);
    update();
}



void ToSourceToggle::refreshIcon()
{
    setIcon();
}


#include "overlaytoggle.moc"
