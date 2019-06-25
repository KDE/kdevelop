/*
 * Copyright 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>
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

namespace KDevelop
{

class CheckerStatusPrivate
{
public:
    int m_maxItems = 0;
    int m_checkedItems = 0;
    QString m_name;

    CheckerStatusPrivate()
    {
    }
};


CheckerStatus::CheckerStatus() :
    d_ptr(new CheckerStatusPrivate())
{
}

CheckerStatus::~CheckerStatus() = default;

QString CheckerStatus::statusName() const
{
    Q_D(const CheckerStatus);

    return i18nc("@info:progress", "Running %1", d->m_name);
}

void CheckerStatus::setCheckerName(const QString &name)
{
    Q_D(CheckerStatus);

    d->m_name = name;
}

void CheckerStatus::setMaxItems(int maxItems)
{
    Q_D(CheckerStatus);

    d->m_maxItems = maxItems;
}

void CheckerStatus::itemChecked()
{
    Q_D(CheckerStatus);

    if (d->m_checkedItems >= d->m_maxItems)
        return;

    d->m_checkedItems = d->m_checkedItems + 1;
    emit showProgress(this, 0, d->m_maxItems, d->m_checkedItems);
}

void CheckerStatus::start()
{
    Q_D(CheckerStatus);

    d->m_checkedItems = 0;
    emit showProgress(this, 0, d->m_maxItems, d->m_checkedItems);
}

void CheckerStatus::stop()
{
    Q_D(CheckerStatus);

    emit clearMessage(this);
    emit showProgress(this, 0, d->m_maxItems, d->m_maxItems);
    emit hideProgress(this);
}

}
