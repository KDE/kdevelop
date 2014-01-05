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

#ifndef KDEVPLATFORM_PLUGIN_TESTCASESWIDGET_H
#define KDEVPLATFORM_PLUGIN_TESTCASESWIDGET_H

#include <QWidget>



class KEditListWidget;

namespace KDevelop
{

/**
 * Assistant page for specifying the list of test cases
 *
 */
class TestCasesPage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QStringList testCases READ testCases WRITE setTestCases)

public:
    explicit TestCasesPage(QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~TestCasesPage();

    /**
     * The name of the new test, as set by the user
     */
    QString name() const;

    /**
     * Returns the list of test case names
     */
    QStringList testCases() const;
    /**
     * Sets the current list of test case names to @p testCases
     */
    void setTestCases(const QStringList& testCases);

Q_SIGNALS:
    void isValid(bool valid);

private:
    class TestCasesPagePrivate* const d;

private Q_SLOTS:
    void identifierChanged(const QString& identifier);
};

}

#endif // KDEVPLATFORM_PLUGIN_TESTCASESWIDGET_H
