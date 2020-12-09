/*
 * This file is part of KDevelop
 * Copyright 2020  Morten Danielsen Volden <mvolden2@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "fixitassistant.h"
#include "fixitaction.h"
#include "fixit.h"

#include <klocalizedstring.h>


namespace shellcheck {

FixitAssistant::FixitAssistant(const QVector<Fixit>& fixits) :
m_title(i18n("ShellCheck Fixit"))
, m_fixits(fixits)
{
}

QString FixitAssistant::title() const
{
    return m_title;
}

void FixitAssistant::createActions()
{
    KDevelop::IAssistant::createActions();

    for (const Fixit& fixit : qAsConst(m_fixits)) {
        addAction(KDevelop::IAssistantAction::Ptr(new FixitAction(fixit)));
    }
}

}
