/***************************************************************************
 *   Copyright 2011 Martin Heide <martin.heide@gmx.net>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
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
private slots:
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
