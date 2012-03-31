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
#include <QFileInfo>
#include <KProcess>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/itestcontroller.h>
#include <language/duchain/duchain.h>

CTestFindJob::CTestFindJob(CTestSuite* suite, QObject* parent): KJob(parent), 
m_suite(suite),
m_process(0)
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
    
    if (!m_suite->arguments().isEmpty())
    {
        tc->addTestSuite(m_suite);
        emitResult();
        return;
    }
    
    QFileInfo info(m_suite->executable().toLocalFile());
    if (info.exists() && info.isExecutable())
    {
        kDebug() << "Starting process to find test cases" << m_suite->name();
        m_process = new KProcess(this);
        m_process->setOutputChannelMode(KProcess::OnlyStdoutChannel);
        m_process->setProgram(info.absoluteFilePath(), QStringList() << "-functions");
        connect (m_process, SIGNAL(finished(int)), SLOT(processFinished()));
        m_process->start();
    }
}

void CTestFindJob::processFinished()
{
    kDebug();
    if (m_process->exitStatus() == KProcess::NormalExit)
    {
        QStringList cases;
        foreach (const QByteArray& line, m_process->readAllStandardOutput().split('\n'))
        {
            QString str = line.trimmed();
            str.remove("()");
            if (!str.isEmpty())
            {
                cases << str;
            }
        }
        m_suite->setTestCases(cases);
    }
    
    m_pendingFiles = m_suite->sourceFiles();
    if (m_pendingFiles.isEmpty())
    {
        KDevelop::ICore::self()->pluginController()->pluginForExtension("org.kdevelop.ITestController")->extension<KDevelop::ITestController>()->addTestSuite(m_suite);
        emitResult();
        return;
    }
    
    foreach (const QString& file, m_pendingFiles)
    {
        KDevelop::DUChain::self()->updateContextForUrl(KDevelop::IndexedString(file), KDevelop::TopDUContext::AllDeclarationsAndContexts, this);
    }
}

void CTestFindJob::updateReady(const KDevelop::IndexedString& document, const KDevelop::ReferencedTopDUContext& context)
{
    kDebug() << m_pendingFiles << document.str();
    m_suite->loadDeclarations(document, context);
    m_pendingFiles.removeAll(document.str());
    
    if (m_pendingFiles.isEmpty())
    {
        KDevelop::ICore::self()->pluginController()->pluginForExtension("org.kdevelop.ITestController")->extension<KDevelop::ITestController>()->addTestSuite(m_suite);
        emitResult();
    }
}


