/***************************************************************************
 *   Copyright (C) 2008 by Peter Penz <peter.penz@gmx.at>                  *
 *             modified by Manuel Breugelmans <mbr.nxi@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "tosourcetoggle.h"
#include "veritas/test.h"

#include <QEvent>
#include <QTimeLine>

#include <KDebug>
#include <KGlobalSettings>
#include <KIconLoader>
#include <KLocale>

using Veritas::ToSourceToggle;
using Veritas::OverlayButton;
using Veritas::Test;

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

bool ToSourceToggle::eventFilter(QObject* obj, QEvent* event)
{
    if ((obj == parent()) && (event->type() == QEvent::Leave)) {
        hide();
    }
    return QAbstractButton::eventFilter(obj, event);
}

void ToSourceToggle::enterEvent(QEvent* event)
{
    QAbstractButton::enterEvent(event);

    // if the mouse cursor is above the selection toggle, display
    // it immediately without fading timer
    m_isHovered = true;
    if (m_fadingTimeLine != 0) {
        m_fadingTimeLine->stop();
    }
    m_fadingValue = 255;
    setToolTip(i18nc("@info:tooltip", "To Source"));
    update();
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

#include "tosourcetoggle.moc"
