/*
 * KDevelop xUnit testing support
 *
 * Copyright 2008 Manuel Breugelmans
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

#include "classskeletontest.h"
#include "kdevtest.h"
#include "../classskeleton.h"

using Veritas::ClassSkeleton;
using Veritas::Test::ClassSkeletonTest;

void ClassSkeletonTest::init()
{
}

void ClassSkeletonTest::cleanup()
{
}

void ClassSkeletonTest::construct()
{
    ClassSkeleton sc;
    KVERIFY(sc.isEmpty());
    KVERIFY(sc.name().isEmpty());
    KVERIFY(sc.methods().isEmpty());
    sc.setName("Foo");
    KOMPARE_("Foo", sc.name());
    KVERIFY(!sc.isEmpty());
}

QTEST_MAIN( ClassSkeletonTest )
#include "classskeletontest.moc"
