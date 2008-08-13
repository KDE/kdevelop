/*
* KDevelop xUnit testing support
* Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
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

#include "methodskeletontest.h"
#include "kdevtest.h"
#include "../methodskeleton.h"

using Veritas::Test::MethodSkeletonTest;
using Veritas::MethodSkeleton;

void MethodSkeletonTest::init()
{
}

void MethodSkeletonTest::cleanup()
{
}

void MethodSkeletonTest::construct()
{
    MethodSkeleton ms;
    KVERIFY(ms.isEmpty());
    ms.setName("foo");
    ms.setBody("int i=0;");
    ms.setArguments("(int j)");
    ms.setReturnType("int");
    KVERIFY(!ms.isEmpty());
    KOMPARE_("foo", ms.name());
    KOMPARE_("int i=0;", ms.body());
    KOMPARE_("(int j)", ms.arguments());
    KOMPARE_("int", ms.returnType());
}

QTEST_MAIN( MethodSkeletonTest )
#include "methodskeletontest.moc"
