/*
 * Copyright 2015 Laszlo Kis-Adam
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "checkerstatus.h"
#include <KLocalizedString>

struct CheckerStatusPrivate
{
    int m_maxItems;
    int m_checkedItems;
    QString m_name;

    CheckerStatusPrivate()
        : m_maxItems(0)
        , m_checkedItems(0)
    {
    }
};

namespace KDevelop
{

CheckerStatus::CheckerStatus() :
    d(new CheckerStatusPrivate())
{
}

CheckerStatus::~CheckerStatus() = default;

QString CheckerStatus::statusName() const
{
    QString s = QStringLiteral("Running ") + d->m_name;

    return i18nc("@info:progress", s.toUtf8().data());
}

void CheckerStatus::setCheckerName(const QString &name)
{
    d->m_name = name;
}

void CheckerStatus::setMaxItems(int maxItems)
{
    d->m_maxItems = maxItems;
}

void CheckerStatus::itemChecked()
{
    if (d->m_checkedItems >= d->m_maxItems)
        return;

    d->m_checkedItems = d->m_checkedItems + 1;
    emit showProgress(this, 0, d->m_maxItems, d->m_checkedItems);
}

void CheckerStatus::start()
{
    d->m_checkedItems = 0;
    emit showProgress(this, 0, d->m_maxItems, d->m_checkedItems);
}

void CheckerStatus::stop()
{
    emit clearMessage(this);
    emit showProgress(this, 0, d->m_maxItems, d->m_maxItems);
    emit hideProgress(this);
}

}
