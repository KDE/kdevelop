/*
 * Copyright 2014  Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "duchaintest.h"

#include <qtest_kde.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testfile.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <language/duchain/parsingenvironment.h>
#include <language/backgroundparser/backgroundparser.h>
#include <interfaces/ilanguagecontroller.h>

QTEST_KDEMAIN(DUChainTest, NoGUI);

using namespace KDevelop;

void DUChainTest::initTestCase()
{
    QVERIFY(qputenv("KDEV_DISABLE_PLUGINS", "kdevcppsupport"));
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void DUChainTest::cleanupTestCase()
{
    TestCore::shutdown();
}

void DUChainTest::testInclude()
{
    TestFile header("int foo() { return 42; }\n", "h");
    // NOTE: header is _not_ explictly being parsed, instead the impl job does that

    TestFile impl("#include \"" + header.url().byteArray() + "\"\n"
                  "int main() { return foo(); }", "cpp", &header);
    impl.parse(TopDUContext::AllDeclarationsContextsAndUses);

    auto implCtx = impl.topContext();
    QVERIFY(implCtx);

    DUChainReadLocker lock;
    QCOMPARE(implCtx->localDeclarations().size(), 1);

    auto headerCtx = DUChain::self()->chainForDocument(header.url());
    QVERIFY(headerCtx);
    QVERIFY(!headerCtx->parsingEnvironmentFile()->needsUpdate());
    QCOMPARE(headerCtx->localDeclarations().size(), 1);

    QVERIFY(implCtx->imports(headerCtx, CursorInRevision(0, 10)));

    Declaration* foo = headerCtx->localDeclarations().first();
    QCOMPARE(foo->uses().size(), 1);
    QCOMPARE(foo->uses().begin().key(), impl.url());
    QCOMPARE(foo->uses().begin()->size(), 1);
    QCOMPARE(foo->uses().begin()->first(), RangeInRevision(1, 20, 1, 23));
}

QByteArray createCode(const QByteArray& prefix, const int functions)
{
    QByteArray code;
    code += "#ifndef " + prefix + "_H\n";
    code += "#define " + prefix + "_H\n";
    for (int i = 0; i < functions; ++i) {
        code += "void myFunc_" + prefix + "(int arg1, char arg2, const char* arg3);\n";
    }
    code += "#endif\n";
    return code;
}

void DUChainTest::testIncludeLocking()
{
    TestFile header1(createCode("Header1", 1000), "h");
    TestFile header2(createCode("Header2", 1000), "h");
    TestFile header3(createCode("Header3", 1000), "h");

    ICore::self()->languageController()->backgroundParser()->setThreadCount(3);

    TestFile impl1("#include \"" + header1.url().byteArray() + "\"\n"
                   "#include \"" + header2.url().byteArray() + "\"\n"
                   "#include \"" + header3.url().byteArray() + "\"\n"
                   "int main() { return 0; }", "cpp");

    TestFile impl2("#include \"" + header2.url().byteArray() + "\"\n"
                   "#include \"" + header1.url().byteArray() + "\"\n"
                   "#include \"" + header3.url().byteArray() + "\"\n"
                   "int main() { return 0; }", "cpp");

    TestFile impl3("#include \"" + header3.url().byteArray() + "\"\n"
                   "#include \"" + header1.url().byteArray() + "\"\n"
                   "#include \"" + header2.url().byteArray() + "\"\n"
                   "int main() { return 0; }", "cpp");

    impl1.parse(TopDUContext::AllDeclarationsContextsAndUses);
    impl2.parse(TopDUContext::AllDeclarationsContextsAndUses);
    impl3.parse(TopDUContext::AllDeclarationsContextsAndUses);

    QVERIFY(impl1.waitForParsed(5000));
    QVERIFY(impl2.waitForParsed(5000));
    QVERIFY(impl3.waitForParsed(5000));

    DUChainReadLocker lock;
    QVERIFY(DUChain::self()->chainForDocument(header1.url()));
    QVERIFY(DUChain::self()->chainForDocument(header2.url()));
    QVERIFY(DUChain::self()->chainForDocument(header3.url()));
}

void DUChainTest::testReparse()
{
    TestFile file("int main() { int i = 42; return i; }", "cpp");
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);

    DeclarationPointer mainDecl;
    DeclarationPointer iDecl;
    for (int i = 0; i < 3; ++i) {
        QVERIFY(file.waitForParsed(500));
        DUChainReadLocker lock;
        QCOMPARE(file.topContext()->childContexts().size(), 1);
        QCOMPARE(file.topContext()->localDeclarations().size(), 1);
        QCOMPARE(file.topContext()->childContexts().first()->localDeclarations().size(), 1);

        if (i) {
            QVERIFY(mainDecl);
            QCOMPARE(mainDecl.data(), file.topContext()->localDeclarations().first());

            QVERIFY(iDecl);
            QCOMPARE(iDecl.data(), file.topContext()->childContexts().first()->localDeclarations().first());
        }
        mainDecl = file.topContext()->localDeclarations().first();
        iDecl = file.topContext()->childContexts().first()->localDeclarations().first();

        QVERIFY(mainDecl->uses().isEmpty());
        QCOMPARE(iDecl->uses().size(), 1);
        QCOMPARE(iDecl->uses().begin()->size(), 1);

        if (i == 1) {
            file.setFileContents("int main()\n{\nfloat i = 13; return i - 5;\n}\n");
        }

        file.parse(TopDUContext::Features(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::ForceUpdateRecursive));
    }
}

#include "duchaintest.moc"
