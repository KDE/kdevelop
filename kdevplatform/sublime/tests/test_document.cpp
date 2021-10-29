/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test_document.h"

#include <QTextEdit>
#include <QTest>
#include <QStandardPaths>

#include <sublime/controller.h>
#include <sublime/tooldocument.h>
#include <sublime/view.h>

using namespace Sublime;

void TestDocument::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestDocument::viewDeletion()
{
    Controller controller;
    Document *doc = new ToolDocument(QStringLiteral("tool"), &controller, new SimpleToolWidgetFactory<QTextEdit>(QStringLiteral("tool")));

    View *view = doc->createView();
    view->widget();
    QCOMPARE(doc->views().count(), 1);

    delete view;
    QCOMPARE(doc->views().count(), 0);
}

QTEST_MAIN(TestDocument)
