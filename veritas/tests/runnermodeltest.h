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

#ifndef VERITAS_RUNNERMODELTEST_H
#define VERITAS_RUNNERMODELTEST_H

#include <QtCore/QObject>
#include <QtTest/QtTest>

namespace Veritas { class RunnerModel; }

namespace Veritas
{
namespace ut {

class RunnerModelStub;

class RunnerModelTest : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void default_();
    void appendResults();
    void changeItems();
    void flags();
    void runItems();
    void errorHandling();
    void countItems();

private:
    void assertColumnHeader(const QVariant& expected, int index);
    void assertDataAt(const QVariant& expected, int row, int column);
    void verifyRowContent(int index);
    void assertSignalValue(QSignalSpy* spy, int expected);

    void fillRows();
    void setUpResultSpies(QMap<QString, QSignalSpy*>& spies);

private:
    RunnerModelStub* model;
};

}
}


#endif // VERITAS_RUNNERMODELTEST_H
