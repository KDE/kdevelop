/*
    SPDX-FileCopyrightText: 2011 Martin Heide <martin.heide@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEST_SHELLBUDDY_H
#define KDEVPLATFORM_TEST_SHELLBUDDY_H

#include <QObject>
#include <QTemporaryDir>

#include <sublime/view.h>
#include "../documentcontroller.h"
#include "../uicontroller.h"

class TestBuddyFinder;

using namespace KDevelop;

class TestShellBuddy: public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testDeclarationDefinitionOrder();
    void testActivation();
    void testDisableBuddies();
    void testDisableOpenAfterCurrent();
    void testDisableAll();

    void testsplitViewBuddies();

private:
    void createFile(const QTemporaryDir& dir, const QString& filename);
    void enableBuddies(bool enable = true);
    void enableOpenAfterCurrent(bool enable = true);

    IDocumentController *m_documentController;
    UiController *m_uiController;
    TestBuddyFinder* m_finder;
};

#endif
