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

#ifndef VERITAS_RUNNERWINDOWTEST_H
#define VERITAS_RUNNERWINDOWTEST_H

#include <QtTest/QtTest>
#include <ui_statuswidget.h>

namespace Veritas
{
class RunnerWindow;
class RunnerModel;
}

namespace Veritas
{
namespace ut {
class RunnerModelStub;
}
}

namespace Veritas
{
namespace it {

class RunnerWindowTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void startItems();
    void deselectItems();

private:
    void assertTestEquals(int item, QVariant col0, QVariant col1, QVariant col2, int result);

private:
    Veritas::RunnerWindow* window;
    Veritas::ut::RunnerModelStub* model;
    Ui::StatusWidget* status;
};

}
}


#endif // VERITAS_RUNNERWINDOWTEST_H
