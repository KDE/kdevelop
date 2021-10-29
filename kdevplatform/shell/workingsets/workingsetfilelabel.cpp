/*
    SPDX-FileCopyrightText: David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "workingsetfilelabel.h"

#include <QMouseEvent>

using namespace KDevelop;

WorkingSetFileLabel::WorkingSetFileLabel()
{
}

void WorkingSetFileLabel::setIsActiveFile(bool active)
{
    if(active)
    {
        ///@todo Use a nicer-looking "blended" highlighting for the active item, like in the area-tabs
        setAutoFillBackground(true);
        setBackgroundRole(QPalette::Highlight);
        setForegroundRole(QPalette::HighlightedText);
    }else{
        setAutoFillBackground(false);
        setBackgroundRole(QPalette::Window);
        setForegroundRole(QPalette::WindowText);
    }
    m_isActive = active;
}

void WorkingSetFileLabel::mouseReleaseEvent(QMouseEvent* ev)
{
    if(ev->button() == Qt::LeftButton)
    {
        ev->accept();
        emit clicked();
        return;
    }

    QLabel::mouseReleaseEvent(ev);
}

bool WorkingSetFileLabel::isActive() const
{
  return m_isActive;
}

void WorkingSetFileLabel::emitClicked()
{
  emit clicked();
}

