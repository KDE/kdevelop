/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_TESTCASESPAGE_H
#define KDEVPLATFORM_PLUGIN_TESTCASESPAGE_H

#include <QWidget>

#include "ipagefocus.h"

namespace KDevelop
{

/**
 * Assistant page for specifying the list of test cases
 *
 */
class TestCasesPage : public QWidget, public IPageFocus
{
    Q_OBJECT
    Q_PROPERTY(QStringList testCases READ testCases WRITE setTestCases)

public:
    explicit TestCasesPage(QWidget* parent = nullptr);
    ~TestCasesPage() override;

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

    void setFocusToFirstEditWidget() override;

Q_SIGNALS:
    void isValid(bool valid);

private:
    class TestCasesPagePrivate* const d;

private Q_SLOTS:
    void identifierChanged(const QString& identifier);
};

}

#endif // KDEVPLATFORM_PLUGIN_TESTCASESPAGE_H
