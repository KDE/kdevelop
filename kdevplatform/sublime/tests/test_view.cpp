/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test_view.h"

#include <QTextEdit>
#include <QTest>
#include <QStandardPaths>

#include <sublime/controller.h>
#include <sublime/tooldocument.h>
#include <sublime/view.h>

using namespace Sublime;

void TestView::widgetDeletion()
{
    Controller controller;
    Document *doc = new ToolDocument(QStringLiteral("tool"), &controller, new SimpleToolWidgetFactory<QTextEdit>(QStringLiteral("tool")));

    View *view = doc->createView();
    //create the widget
    const auto* const widget = view->initializeWidget(nullptr);
    QVERIFY(widget);
    QCOMPARE(view->widget(), widget);
    QCOMPARE(view->widget()->metaObject()->className(), "QTextEdit");

    //delete the widget and check that view knows about that
    delete view->widget();
    QVERIFY(!view->widget());
}

class Test: public View {
Q_OBJECT
public:
    explicit Test(Document *doc): View(doc) {}
};

class TestDocument: public Document {
Q_OBJECT
public:
    explicit TestDocument(Controller *controller): Document(QStringLiteral("TestDocument"), controller) {}
    QString documentType() const override { return QStringLiteral("Test"); }
    QString documentSpecifier() const override { return QString(); }
protected:
    QWidget *createViewWidget(QWidget *parent = nullptr) override { return new QWidget(parent); }
    View *newView(Document *doc) override { return new Test(doc); }
};

void TestView::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestView::viewReimplementation()
{
    Controller controller;
    Document *doc = new TestDocument(&controller);
    View *view = doc->createView();
    QVERIFY(dynamic_cast<Test*>(view) != nullptr);
}

QTEST_MAIN(TestView)

#include "test_view.moc"
#include "moc_test_view.cpp"
