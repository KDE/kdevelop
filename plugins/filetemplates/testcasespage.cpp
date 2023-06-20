/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testcasespage.h"

#include "ui_testcases.h"

#include <KEditListWidget>

using namespace KDevelop;

class KDevelop::TestCasesPagePrivate
{
public:
    Ui::TestCasesPage* ui;
};

TestCasesPage::TestCasesPage(QWidget* parent)
: QWidget(parent)
, d(new TestCasesPagePrivate)
{
    d->ui = new Ui::TestCasesPage();
    d->ui->setupUi(this);

    d->ui->testCasesLabel->setBuddy(d->ui->keditlistwidget->lineEdit());

    connect(d->ui->identifierLineEdit, &QLineEdit::textChanged, this, &TestCasesPage::identifierChanged);
}

TestCasesPage::~TestCasesPage()
{
    delete d->ui;
    delete d;
}

QString TestCasesPage::name() const
{
    return d->ui->identifierLineEdit->text();
}

void TestCasesPage::setTestCases(const QStringList& testCases)
{
    d->ui->keditlistwidget->setItems(testCases);
}

QStringList TestCasesPage::testCases() const
{
    return d->ui->keditlistwidget->items();
}

void TestCasesPage::setFocusToFirstEditWidget()
{
    d->ui->identifierLineEdit->setFocus();
}

void TestCasesPage::identifierChanged(const QString& identifier)
{
    emit isValid(!identifier.isEmpty());
}

#include "moc_testcasespage.cpp"
