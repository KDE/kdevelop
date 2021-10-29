/*
    SPDX-FileCopyrightText: 2011 Martin Heide <martin.heide@gmx.net>
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SHELLBUDDYTEST_H
#define SHELLBUDDYTEST_H

#include <QObject>
#include <QDir>

namespace Sublime {
class View;
class Controller;
}

namespace KDevelop {
class IUiController;
class IDocumentController;
}

using namespace KDevelop;

class TestBuddies: public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    void testDeclarationDefinitionOrder();
    void testMultiDotFilenames();
    void testActivation();
    void testDisableBuddies();
    void testDisableOpenAfterCurrent();
    void testDisableAll();

    void testMultipleFolders();
    void testSplitViewBuddies();

    void testDUChainBuddy();
    void testDUChainBuddyVote();

private:
    void verifyFilename(Sublime::View *view, const QString& endOfFilename);
    void createFile(const QDir& dir, const QString& filename);
    void enableBuddies(bool enable = true);
    void enableOpenAfterCurrent(bool enable = true);

    IDocumentController *m_documentController;
    IUiController *m_uiController;
    Sublime::Controller *m_sublimeController;
};

#endif
