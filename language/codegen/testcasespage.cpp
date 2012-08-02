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

#include <KEditListWidget>
#include <QLayout>
#include <QVBoxLayout>

TestCasesPage::TestCasesPage (QWidget* parent, Qt::WindowFlags f) : QWidget (parent, f)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    ui = new KEditListWidget(this);
    layout->addWidget(ui);
    setLayout(layout);
}

TestCasesPage::~TestCasesPage()
{

}

void TestCasesPage::setTestCases (const QStringList& testCases)
{
    ui->setItems(testCases);
}

QStringList TestCasesPage::testCases() const
{
    return ui->items();
}
