/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Kris Wong <kris.p.wong@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "icore.h"

namespace KDevelop {

ICore *ICore::m_self = nullptr;

ICore::ICore(QObject *parent)
    : QObject(parent)
{
    Q_ASSERT(m_self == nullptr);
    m_self = this;
}

ICore::~ICore()
{
    m_self = nullptr;
}

ICore *ICore::self()
{
    return m_self;
}

}

