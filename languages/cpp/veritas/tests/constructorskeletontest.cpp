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
#include "constructorskeletontest.h"
#include "kdevtest.h"
#include "../constructorskeleton.h"

using Veritas::Test::ConstructorSkeletonTest;
using Veritas::ConstructorSkeleton;

void ConstructorSkeletonTest::init()
{
}

void ConstructorSkeletonTest::cleanup()
{
}

void ConstructorSkeletonTest::instantiate()
{
    ConstructorSkeleton cs;
    KVERIFY(cs.isEmpty());
    cs.setName("Foo");
    cs.setBody("int i=0;");
    cs.addInitializer("m_i(0)");
    cs.addInitializer("m_j(1)");
    KOMPARE("Foo", cs.name());
    KOMPARE("int i=0;", cs.body());
    KOMPARE(QStringList() << "m_i(0)" << "m_j(1)", cs.initializerList());
}

QTEST_MAIN( ConstructorSkeletonTest )
#include "constructorskeletontest.moc"
