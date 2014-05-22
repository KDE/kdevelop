/*************************************************************************************
 *  Copyright (C) 2013 by Milian Wolff <mail@milianw.de>                             *
 *  Copyright (C) 2013 Olivier de Gaalon <olivier.jg@gmail.com>                      *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "test_files.h"

#include <qtest_kde.h>

#include <language/duchain/duchain.h>
#include <language/codegen/coderepresentation.h>
#include <language/backgroundparser/backgroundparser.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/json/declarationvalidator.h>

#include "testfilepaths.h"

//Include all used json tests, otherwise "Test not found"
#include <tests/json/jsondeclarationtests.h>
#include <tests/json/jsonducontexttests.h>
#include <tests/json/jsontypetests.h>
#include <interfaces/ilanguagecontroller.h>

// #include "cppjsontests.h"

using namespace KDevelop;

QTEST_KDEMAIN(TestFiles, GUI)

void TestFiles::initTestCase()
{
  AutoTestShell::init();
  TestCore::initialize(KDevelop::Core::NoUi);
  DUChain::self()->disablePersistentStorage();
  Core::self()->languageController()->backgroundParser()->setDelay(0);
  CodeRepresentation::setDiskChangesForbidden(true);
}

void TestFiles::cleanupTestCase()
{
  TestCore::shutdown();
}

void TestFiles::testFiles_data()
{
  QTest::addColumn<QString>("fileName");
  const QString testDirPath = TEST_FILES_DIR;
  QStringList files = QDir(testDirPath).entryList(QStringList() << "*.js" << "*.qml", QDir::Files);
  foreach (QString file, files) {
    QTest::newRow(file.toUtf8()) << QString(testDirPath + "/" + file);
  }
}

void TestFiles::testFiles()
{
  QFETCH(QString, fileName);
  const IndexedString indexedFileName(fileName);
  ReferencedTopDUContext top =
      DUChain::self()->waitForUpdate(indexedFileName, KDevelop::TopDUContext::AllDeclarationsContextsAndUses);

  while ( ICore::self()->languageController()->backgroundParser()->queuedCount() != 0 ) {
      QTest::qWait(10);
  }

  QVERIFY(top);
  DUChainReadLocker lock;
  DeclarationValidator validator;
  top->visit(validator);
  QVERIFY(validator.testsPassed());
}

#include "test_files.moc"
