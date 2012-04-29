/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "outputviewtest.h"

#include <QTest>

#include <qtest_kde.h>


QTEST_KDEMAIN(OutputViewTest, NoGUI)

void OutputViewTest::initTestCase()
{
//   AutoTestShell::init();
//   TestCore::initialize(Core::NoUi);
//   TestCore* core = dynamic_cast<TestCore*>(TestCore::self());
// 
//   DUChain::self()->disablePersistentStorage();
// 
//   TestLanguageController* langController = new TestLanguageController(core);
//   core->setLanguageController(langController);
//   langController->backgroundParser()->setThreadCount(4);
// 
//   TestLanguageSupport* testLang = new TestLanguageSupport();
//   connect(testLang, SIGNAL(parseJobCreated(KDevelop::ParseJob*)),
//           &m_jobPlan, SLOT(parseJobCreated(KDevelop::ParseJob*)));
//   langController->addTestLanguage(testLang, QStringList() << "text/plain");
}

void OutputViewTest::cleanupTestCase()
{
  //TestCore::shutdown();
}

void OutputViewTest::init()
{
    //m_jobPlan.clear();
}


