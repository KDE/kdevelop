/*
    SPDX-FileCopyrightText: 2026 Yin Zhijie <depressedjie@outlook.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "test_navigationcontext.h"

#include <language/duchain/duchainlock.h>
#include <language/duchain/navigation/abstractdeclarationnavigationcontext.h>
#include <language/duchain/problem.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <tests/testfile.h>

#include <QLoggingCategory>
#include <QTest>

using namespace KDevelop;

namespace {
class TestDeclarationNavigationContext : public AbstractDeclarationNavigationContext
{
public:
    using AbstractDeclarationNavigationContext::AbstractDeclarationNavigationContext;
    using AbstractDeclarationNavigationContext::declarationDetails;
};
}

QTEST_GUILESS_MAIN(TestNavigationContext)

void TestNavigationContext::initTestCase()
{
    QLoggingCategory::setFilterRules(
        QStringLiteral("*.debug=false\ndefault.debug=true\nkdevelop.plugins.clang.debug=true\n"));
    QVERIFY(qputenv("KDEV_CLANG_DISPLAY_DIAGS", "1"));
    AutoTestShell::init({QStringLiteral("kdevclangsupport")});
    TestCore::initialize(Core::NoUi);
}

void TestNavigationContext::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestNavigationContext::testDeclarationDetailsVirtualFinalAbstract()
{
    TestFile file(QStringLiteral(R"(
        class Base {
        public:
            virtual bool isAbstract() = 0;
            virtual bool onlyVirtual();
        };

        class Derived : public Base {
            virtual ~Derived();
            virtual bool isFinal() final;
        };

        class DestructorBase {
        public:
            virtual ~DestructorBase() = default;
        };

        class FinalDestructor final : public DestructorBase {
            ~FinalDestructor() final;
        };
    )"),
                  QStringLiteral("cpp"));
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);
    QVERIFY(file.waitForParsed(1000));
    {
        DUChainReadLocker lock;
        const auto top = file.topContext();
        QVERIFY(top);
        QVERIFY(top->problems().isEmpty());

        const auto onlyVirtualDecls = top->findDeclarations(QualifiedIdentifier(u"Base::onlyVirtual"));
        QCOMPARE(onlyVirtualDecls.size(), 1);
        const DeclarationPointer onlyVirtualDecl(onlyVirtualDecls.first());

        const auto isAbstractDecls = top->findDeclarations(QualifiedIdentifier(u"Base::isAbstract"));
        QCOMPARE(isAbstractDecls.size(), 1);
        const DeclarationPointer isAbstractDecl(isAbstractDecls.first());

        const auto isFinalDecls = top->findDeclarations(QualifiedIdentifier(u"Derived::isFinal"));
        QCOMPARE(isFinalDecls.size(), 1);
        const DeclarationPointer isFinalDecl(isFinalDecls.first());

        const auto finalDestructorDecls =
            top->findDeclarations(QualifiedIdentifier(u"FinalDestructor::~FinalDestructor"));
        QCOMPARE(finalDestructorDecls.size(), 1);
        const DeclarationPointer finalDestructorDecl(finalDestructorDecls.first());

        const QStringList onlyVirtualDetails = TestDeclarationNavigationContext::declarationDetails(onlyVirtualDecl);
        QVERIFY(onlyVirtualDetails.contains(QStringLiteral("virtual")));
        QVERIFY(!onlyVirtualDetails.contains(QStringLiteral("abstract")));
        QVERIFY(!onlyVirtualDetails.contains(QStringLiteral("final")));

        const QStringList isAbstractDetails = TestDeclarationNavigationContext::declarationDetails(isAbstractDecl);
        QVERIFY(isAbstractDetails.contains(QStringLiteral("abstract")));
        QVERIFY(!isAbstractDetails.contains(QStringLiteral("virtual")));
        QVERIFY(!isAbstractDetails.contains(QStringLiteral("final")));

        const QStringList isFinalDetails = TestDeclarationNavigationContext::declarationDetails(isFinalDecl);
        QVERIFY(isFinalDetails.contains(QStringLiteral("final")));
        QVERIFY(!isFinalDetails.contains(QStringLiteral("virtual")));
        QVERIFY(!isFinalDetails.contains(QStringLiteral("abstract")));

        const QStringList finalDestructorDetails =
            TestDeclarationNavigationContext::declarationDetails(finalDestructorDecl);
        QVERIFY(finalDestructorDetails.contains(QStringLiteral("final")));
        QVERIFY(!finalDestructorDetails.contains(QStringLiteral("virtual")));
        QVERIFY(!finalDestructorDetails.contains(QStringLiteral("abstract")));
    }
}

#include "moc_test_navigationcontext.cpp"
