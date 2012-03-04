/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "ctestfindjob.h"
#include "ctestsuite.h"
#include <QTimer>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/itestcontroller.h>

CTestFindJob::CTestFindJob(CTestSuite* suite, QObject* parent): KJob(parent), 
m_suite(suite)
{

}

void CTestFindJob::start()
{
    QTimer::singleShot(0, this, SLOT(findTestCases()) );
}

void CTestFindJob::findTestCases()
{
    // Test suite names must be unique within a project
    // If we have a clash, it's probably an update and the old one should be removed. 
    KDevelop::ITestController* tc = KDevelop::ICore::self()->pluginController()->pluginForExtension("org.kdevelop.ITestController")->extension<KDevelop::ITestController>();
    if (KDevelop::ITestSuite* existingSuite = tc->findTestSuite(m_suite->project(), m_suite->name()))
    {
        tc->removeTestSuite(existingSuite);
    }
    
    m_suite->loadCases();
    tc->addTestSuite(m_suite);
    emitResult();
}
