/*
    SPDX-FileCopyrightText: 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
