/*
 * This file is part of KDevelop
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

#ifndef TESTCASE_H
#define TESTCASE_H

#include <veritas/test.h>
#include "testcommand.h"
#include <QtCore/QString>

/*!
 * Item in the test tree that contains TestCommands
 *
 * This class removes bad duplication between the Check
 * & CppUnit plugins. It might be completely irrelevant for
 * other frameworks.
 */
class TestCase : public Veritas::Test
{
Q_OBJECT
public:
    TestCase(const QString& name, Veritas::Test* parent);
    virtual ~TestCase();
    TestCommand* child(int i) const;
};


#endif // TESTCASE_H
