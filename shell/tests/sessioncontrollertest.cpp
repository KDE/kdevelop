/***************************************************************************
 *   Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>                 *
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

#include "sessioncontrollertest.h"

#include <QtGui>
#define QT_GUI_LIB 1
#include <QtTest/QtTest>

#include <qtest_kde.h>
#include <tests/common/autotestshell.h>

#include "../core.h"
#include "../sessioncontroller.h"
#include "../session.h"

using KDevelop::SessionController;
using KDevelop::ISession;
using KDevelop::Core;

using QTest::kWaitForSignal;

////////////////////// Fixture ///////////////////////////////////////////////

void SessionControllerTest::initTestCase()
{
    AutoTestShell::init();
    Core::initialize();
    m_core = Core::self();
}

void SessionControllerTest::init()
{
    m_sessionCtrl = m_core->sessionControllerInternal();
}

void SessionControllerTest::cleanup()
{
}


QTEST_KDEMAIN( SessionControllerTest, GUI)
#include "sessioncontrollertest.moc"