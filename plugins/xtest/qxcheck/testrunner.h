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

#ifndef QXCHECK_TESTRUNNER_H
#define QXCHECK_TESTRUNNER_H

#include "checkexport.h"

#include <QWidget>
#include <QFileInfo>

namespace QxRunner { class Runner; }

namespace QxCheck
{
class CheckModel;

class QXCHECK_EXPORT TestRunner
{
public:

    TestRunner();
    virtual ~TestRunner();
    void registerTests(const QFileInfo& exe);
    QWidget* spawn();

private:
    // Copy and assignment not supported.
    TestRunner(const TestRunner&);
    TestRunner& operator=(const TestRunner&);

private:
    QxRunner::Runner* m_runner;
    CheckModel*     m_model;
};

} // namespace QxCheck

#endif // TESTRUNNER_H
