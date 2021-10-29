/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "holdupdates.h"
#include <QWidget>

namespace Sublime {
    
HoldUpdates::HoldUpdates(QWidget* w)
    : m_wasupdating(w->updatesEnabled()), m_done(false), m_w(w)
{
    if(m_wasupdating) w->setUpdatesEnabled(false);
}

void HoldUpdates::stop()
{
    if(!m_done)
        m_w->setUpdatesEnabled(m_wasupdating);
    m_done=true;
}

HoldUpdates::~HoldUpdates()
{
    stop();
}

}

