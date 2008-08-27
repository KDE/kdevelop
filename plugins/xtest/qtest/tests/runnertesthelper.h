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

#ifndef VERITAS_RUNNERTESTHELPER_H
#define VERITAS_RUNNERTESTHELPER_H

#include <QStringList>
#include <veritas/testresult.h>

namespace Veritas
{

class Test;
class RunnerWindow;
class RunnerTestHelper : public QObject
{
Q_OBJECT
public:
    RunnerTestHelper();
    /*! setup the GUI */
    void initializeGUI();
    void cleanupGUI();

    /*! the test tree root*/
    void setRoot(Test* root);
    /*! run the items in the tree. blocking */
    void runTests();
    /*! compares the actual test tree in the model against a serialized
    version in @p testTreeContents
    eg "0 suite1"
       "0 0 fakeqtest1"
       "0 0 0 cmd1"
       "0 0 1 x"
       "0 1 x"
       "1 x" */
    void verifyTestTree(QStringList testTreeContents);
    void verifyResultItems(QList<QStringList> expected);
    void verifyTestStates(QMap<QString, Veritas::TestState>, Veritas::Test* root);

private slots:
    void triggerRunAction();

private:
    void verifyTest(const QVariant& expected, int lvl0, int lvl1, int lvl2);
    void nrofMessagesEquals(int num);
    void checkResultItem(int num, const QStringList& item);

private:
    RunnerWindow* m_window;
};

} // end namespace Veritas

#endif // VERITAS_RUNNERTESTHELPER_H
