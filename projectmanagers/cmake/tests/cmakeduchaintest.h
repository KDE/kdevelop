/* KDevelop CMake Support
 *
 * Copyright 2008 Aleix Pol Gonzalez <aleixpol@gmail.com>
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

#ifndef CMAKEDUCHAINTEST_H
#define CMAKEDUCHAINTEST_H

#include <QtTest/QtTest>
#include <language/duchain/topducontext.h>

/**
 * A test for the CMake DUChain builder.
 * @author Aleix Pol <aleixpol@gmail.com>
 */


class CMakeDUChainTest : public QObject
{
    Q_OBJECT
    private slots:
        void initTestCase();
        void cleanupTestCase();

        void testUses_data();
        void testUses();
        void testDUChainWalk_data();
        void testDUChainWalk();
};

#endif
