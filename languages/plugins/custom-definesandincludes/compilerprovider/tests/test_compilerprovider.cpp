/*
 * This file is part of KDevelop
 *
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
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
 *
 */

#include "test_compilerprovider.h"

#include <QtTest/QtTest>

#include <qtest_kde.h>

#include <interfaces/iplugincontroller.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <language/interfaces/idefinesandincludesmanager.h>
#include "../icompilerprovider.h"

using KDevelop::ICore;
using KDevelop::TestCore;
using KDevelop::AutoTestShell;
using KDevelop::IDefinesAndIncludesManager;

ICompilerProvider* compilerProvider()
{
    auto cp = KDevelop::ICore::self()->pluginController()->pluginForExtension("org.kdevelop.ICompilerProvider");
    if (!cp || !cp->extension<ICompilerProvider>()) {
        return {};
    }

    return cp->extension<ICompilerProvider>();
}


void TestCompilerProvider::cleanupTestCase()
{
    if (compilerProvider()) {
        for (auto c : compilerProvider()->compilers()) {
            compilerProvider()->unregisterCompiler(c);
        }
    }

    TestCore::shutdown();
}

void TestCompilerProvider::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(KDevelop::Core::NoUi);
}

void TestCompilerProvider::testRegisterCompiler()
{
    if (!compilerProvider()) {
        return;
    }

    auto cp = compilerProvider();
    auto cf = compilerProvider()->compilerFactories();
    for (int i = 0 ; i < cf.size(); ++i) {
        auto compiler = cf[i]->createCompiler(QString::number(i), QString::number(i));
        QVERIFY(cp->registerCompiler(compiler));
        QVERIFY(!cp->registerCompiler(compiler));
        QVERIFY(cp->compilers().contains(compiler));
    }
}

void TestCompilerProvider::testSetCompiler()
{
    if (!compilerProvider()) {
        return;
    }

    for (auto c : compilerProvider()->compilers()) {
        compilerProvider()->setCompiler(nullptr, c);
        QCOMPARE(compilerProvider()->currentCompiler(nullptr), c);
    }
}

void TestCompilerProvider::testCompilerIncludesAndDefines()
{
    if (!compilerProvider()) {
        return;
    }

    for (auto c : compilerProvider()->compilers()) {
        if (!c->editable() && !c->path().isEmpty()) {
            compilerProvider()->setCompiler(nullptr, c);
            QVERIFY(!c->defines().isEmpty());
            QVERIFY(!c->includes().isEmpty());
            QCOMPARE(dynamic_cast<IDefinesAndIncludesManager::Provider*>(compilerProvider())->defines(nullptr), c->defines());
            QCOMPARE(dynamic_cast<IDefinesAndIncludesManager::Provider*>(compilerProvider())->includes(nullptr), c->includes());
        }
    }
}

QTEST_KDEMAIN(TestCompilerProvider, NoGUI)
