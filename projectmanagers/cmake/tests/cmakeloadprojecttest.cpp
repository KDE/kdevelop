/***************************************************************************
 *   Copyright 2009 Andreas Pakulat <apaku@gmx.de>                         *
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

#include "cmakeloadprojecttest.h"

#include <qtest_kde.h>
#include <QtTest/QtTest>

#include <kactioncollection.h>
#include <kxmlguifactory.h>
#include <kdebug.h>

#include <interfaces/iprojectcontroller.h>

#include <tests/autotestshell.h>
#include <tests/kdevsignalspy.h>

QTEST_KDEMAIN(CMakeLoadProjectTest, NoGUI)

void CMakeLoadProjectTest::initTestCase() 
{
    AutoTestShell::init();
    m_testcore = new KDevelop::TestCore();
    m_testcore->initialize( KDevelop::Core::NoUi );
}

void CMakeLoadProjectTest::cleanupTestCase()
{
    m_testcore->cleanup();
    delete m_testcore;
}

void CMakeLoadProjectTest::testOpenProject()
{
    KDevSignalSpy* spy = new KDevSignalSpy( m_testcore->projectController(), SIGNAL(projectOpened( KDevelop::IProject* ) ) );
    m_testcore->projectController()->openProject( KUrl( "/home/andreas/temp/testcmakekde4/testcmakekde4.kdev4" ) );
    bool gotSignal = spy->wait(60000);
    QVERIFY2(gotSignal, "Timeout while waiting for project opening");
}

#include "cmakeloadprojecttest.moc"

