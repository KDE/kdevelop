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

#ifndef VERITAS_ITESTRUNNER_H
#define VERITAS_ITESTRUNNER_H

#include "veritas/veritasexport.h"

class QWidget;

namespace Veritas
{
class ITest;

/*!
 * base for test runner toolviews/plugins.
 * implemented in TestRunnerToolView
 */
class VERITAS_EXPORT ITestRunner
{
public:
    ITestRunner();
    virtual ~ITestRunner();

    /*!
     * Create a new test runner widget
     */
    virtual QWidget* spawnWindow() = 0;

protected:

    /*!
     * Reload the test tree.
     * To be implemented by concrete plugins.
     */
    virtual ITest* registerTests() = 0;
};

}

#endif // VERITAS_ITESTRUNNER_H
