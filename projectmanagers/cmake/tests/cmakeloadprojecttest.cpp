/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
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

#include "cmakeloadprojecttest.h"

#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/topducontext.h>


#include "cmListFileLexer.h"
#include "cmakelistsparser.h"
#include "cmakeprojectvisitor.h"
#include "cmakeast.h"

QTEST_MAIN( CMakeLoadProjectTest )

using namespace KDevelop;

CMakeLoadProjectTest::CMakeLoadProjectTest()
{
}

CMakeLoadProjectTest::~CMakeLoadProjectTest()
{
}

void CMakeLoadProjectTest::testTinyCMakeProject()
{
    KDevelop::ReferencedTopDUContext m_fakeContext;
    {
        DUChainWriteLocker lock(DUChain::lock());
        m_fakeContext = new TopDUContext(IndexedString("test"), SimpleRange(0,0,0,0));
        DUChain::self()->addDocumentChain(m_fakeContext);
    }

    QString projectfile = CMAKE_TESTS_PROJECTS_DIR"/tiny_project/CMakeLists.txt";
    CMakeFileContent code=CMakeListsParser::readCMakeFile(projectfile);
    QVERIFY(code.count() != 0);

    MacroMap mm;
    VariableMap vm;
    CacheValues cv;
    vm.insert("CMAKE_CURRENT_SOURCE_DIR", QStringList("."));

    CMakeProjectVisitor v(projectfile, m_fakeContext);
    v.setVariableMap(&vm);
    v.setMacroMap(&mm);
    v.setCacheValues(&cv);
//     v.setModulePath();
    v.walk(code, 0);

    ReferencedTopDUContext ctx=v.context();
    QVERIFY(ctx);

}

#include "cmakeloadprojecttest.moc"

