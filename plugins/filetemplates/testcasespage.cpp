/*
 * This file is part of KDevelop
 * Copyright 2012 Miha Čančula <miha@noughmad.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "testcasespage.h"

#include "ui_testcases.h"

#include <KEditListWidget>
#include <kwidgetsaddons_version.h>

#include <QVBoxLayout>

using namespace KDevelop;

class KDevelop::TestCasesPagePrivate
{
public:
    Ui::TestCasesPage* ui;
};

TestCasesPage::TestCasesPage(QWidget* parent, Qt::WindowFlags f)
: QWidget (parent, f)
, d(new TestCasesPagePrivate)
{
    d->ui = new Ui::TestCasesPage();
    d->ui->setupUi(this);

    d->ui->testCasesLabel->setBuddy(d->ui->keditlistwidget->lineEdit());

#if KWIDGETSADDONS_VERSION < QT_VERSION_CHECK(5,32,0)
    // workaround for KEditListWidget bug:
    // ensure keyboard focus is returned to edit line
    connect(d->ui->keditlistwidget, &KEditListWidget::added,
            d->ui->keditlistwidget->lineEdit(),
            static_cast<void(QWidget::*)()>(&QWidget::setFocus));
    connect(d->ui->keditlistwidget, &KEditListWidget::removed,
            d->ui->keditlistwidget->lineEdit(),
            static_cast<void(QWidget::*)()>(&QWidget::setFocus));
#endif

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
