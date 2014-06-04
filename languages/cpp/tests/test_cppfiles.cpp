/* This file is part of KDevelop
   Copyright 2012 Olivier de Gaalon <olivier.jg@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <qtest_kde.h>
#include <language/duchain/duchain.h>
#include <language/duchain/dumpchain.h>
#include <language/codegen/coderepresentation.h>
#include <language/backgroundparser/backgroundparser.h>
#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/json/declarationvalidator.h>
#include "test_cppfiles.h"
#include "testfilepaths.h"

//Include all used json tests, otherwise "Test not found"
#include <tests/json/jsondeclarationtests.h>
#include <tests/json/jsonducontexttests.h>
#include <tests/json/jsontypetests.h>
#include <interfaces/ilanguagecontroller.h>
#include "cppjsontests.h"

using namespace KDevelop;
using namespace Cpp;

QTEST_KDEMAIN(TestCppFiles, GUI)

///Need to override visit(Declaration*) so that instantiated declarations
///Don't get tested with the data used by the uninstantiated declaration
class CppDeclarationValidator : public DeclarationValidator
{
public:
  void visit(DUContext* context)
  {
      KDevelop::DeclarationValidator::visit(context);
  }
  void visit(Declaration* declaration)
  {
    TemplateDeclaration *templateDecl = dynamic_cast<TemplateDeclaration*>(declaration);
    if (templateDecl && (templateDecl->instantiatedFrom() && !templateDecl->specializedFrom().isValid()))
      return; //Instantiations are tested on request

    KDevelop::DeclarationValidator::visit(declaration);
  }
};

void TestCppFiles::initTestCase()
{
  //Intentionally load all plugins, otherwise for some reasons kdevcompilerprovider won't be loaded even if write it name here...
  AutoTestShell::init();
  TestCore::initialize(KDevelop::Core::NoUi);
  DUChain::self()->disablePersistentStorage();
  Core::self()->languageController()->backgroundParser()->setDelay(0);
  CodeRepresentation::setDiskChangesForbidden(true);
}
void TestCppFiles::cleanupTestCase()
{
  TestCore::shutdown();
}

void TestCppFiles::testFiles_data()
{
  QTest::addColumn<QString>("fileName");
  const QString testDirPath = CPP_TEST_FILES_DIR;
  QStringList files = QDir(testDirPath).entryList(QStringList() << "*.cpp", QDir::Files);
  foreach (QString file, files) {
    QTest::newRow(file.toUtf8()) << QString(testDirPath + "/" + file);
  }
}
void TestCppFiles::testFiles()
{
  QFETCH(QString, fileName);
  IndexedString indexedFileName = IndexedString(fileName);
  ReferencedTopDUContext top =
      DUChain::self()->waitForUpdate(indexedFileName, KDevelop::TopDUContext::AllDeclarationsContextsAndUses);
  QVERIFY(top);
  DUChainReadLocker lock;
  CppDeclarationValidator validator;
  if (QProcessEnvironment::systemEnvironment().contains("DUMP_DUCONTEXTS")) {
    dumpDUContext(top);
  }
  top->visit(validator);
  QVERIFY(validator.testsPassed());
}

#include "test_cppfiles.moc"
