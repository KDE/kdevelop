/***************************************************************************
 *   Copyright 2008 Alexander Dymo <adymo@kdevelop.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#include "shelldocumentoperationtest.h"

#include <tests/common/kdevtest.h>
#include <QtTest/QtTest>

#include "documentcontroller.h"

void ShellDocumentOperationTest::init()
{
    AutoTestShell::init();
    KDevelop::Core::initialize();
}

void ShellDocumentOperationTest::cleanup()
{
}

void ShellDocumentOperationTest::testOpenDocumentFromText()
{
    //open some docs
    IDocumentController *documentController = Core::self()->documentController();
    documentController->openDocumentFromText("Test1");
    QTest::qWait(10000);

    //test that we have this document in the list, signals are emitted and so on
    QCOMPARE(documentController->openDocuments().count(), 1);
}

KDEVTEST_MAIN(ShellDocumentOperationTest)
#include "shelldocumentoperationtest.moc"
