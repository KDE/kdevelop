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

#ifndef TESTCASESWIDGET_H
#define TESTCASESWIDGET_H

#include <QtGui/QWidget>

class KEditListWidget;

class TestCasesWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QStringList testCases READ testCases WRITE setTestCases)

public:
    explicit TestCasesWidget (QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~TestCasesWidget();

    QStringList testCases() const;
    void setTestCases(const QStringList& testCases);

private:
    KEditListWidget* ui;

};

#endif // TESTCASESWIDGET_H
