/*
    SPDX-FileCopyrightText: 2012 Morten Danielsen Volden <mvolden2@gmail.com>
    SPDX-FileCopyrightText: 2016 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ifilterstrategy.h"

namespace KDevelop
{


IFilterStrategy::IFilterStrategy()
{
}

IFilterStrategy::~IFilterStrategy()
{
}

IFilterStrategy::Progress IFilterStrategy::progressInLine(const QString& line)
{
    Q_UNUSED(line);
    return {};
}

}
