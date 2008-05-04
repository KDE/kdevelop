/* KDevelop xUnit plugin
 *
 * Copyright 2006 systest.ch <qxrunner@systest.ch>
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

/*!
 * \file  proxymodelcommon.cpp
 *
 * \brief Implements class ProxyModelCommon.
 */

#include "proxymodelcommon.h"

namespace QxRunner
{

ProxyModelCommon::ProxyModelCommon()
{
    m_active = true;
}

ProxyModelCommon::~ProxyModelCommon()
{

}

bool ProxyModelCommon::isActive() const
{
    return m_active;
}

void ProxyModelCommon::setActive(bool active)
{
    m_active = active;
}

QBitArray ProxyModelCommon::enabledColumns() const
{
    return m_enabledColumns;
}

void ProxyModelCommon::setEnabledColumns(const QBitArray& enabledColumns)
{
    m_enabledColumns = enabledColumns;
}

bool ProxyModelCommon::isColumnEnabled(int column) const
{
    if (column >= 0 && column < m_enabledColumns.size()) {
        return m_enabledColumns[column];
    } else {
        return false;
    }
}

} // namespace

