/* KDevelop xUnit plugin
 *
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

#include <proxymodelcommon.h>
#include <QString>

#include "kasserts.h"
#include "proxymodelcommontest.h"

using QxRunner::ProxyModelCommon;

void ProxyModelCommonTest::init()
{
    proxy = new ProxyModelCommon();
}

void ProxyModelCommonTest::cleanup()
{
    delete proxy;
}

// test command
void ProxyModelCommonTest::active()
{
    KVERIFY(proxy->isActive());
    proxy->setActive(false);
    KVERIFY(!proxy->isActive());
    proxy->setActive(true);
    KVERIFY(proxy->isActive());
}

// test command
void ProxyModelCommonTest::defaultEnabled()
{
    // default evrything disabled
    QBitArray default_ = proxy->enabledColumns();
    KOMPARE(0, default_.size());
    KVERIFY(!proxy->isColumnEnabled(0));
    KVERIFY(!proxy->isColumnEnabled(1));
    KVERIFY(!proxy->isColumnEnabled(2));
}

// test command
void ProxyModelCommonTest::enabledColumns()
{

    QBitArray cols(3);
    cols.setBit(0);
    cols.clearBit(1);
    cols.setBit(2);
    proxy->setEnabledColumns(cols);

    KVERIFY(proxy->isColumnEnabled(0));
    KVERIFY(!proxy->isColumnEnabled(1));
    KVERIFY(proxy->isColumnEnabled(2));
}

QTEST_MAIN( ProxyModelCommonTest );
