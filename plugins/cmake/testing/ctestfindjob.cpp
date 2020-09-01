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
#include <debug.h>

#include <interfaces/icore.h>
#include <interfaces/itestcontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/duchain/duchain.h>
#include <language/backgroundparser/backgroundparser.h>

#include <KLocalizedString>

CTestFindJob::CTestFindJob(CTestSuite* suite, QObject* parent)
: KJob(parent)
, m_suite(suite)
{
    qCDebug(CMAKE) << "Created a CTestFindJob";
    setObjectName(i18n("Parse test suite %1", suite->name()));
    setCapabilities(Killable);
}

void CTestFindJob::start()
{
    qCDebug(CMAKE);
    QMetaObject::invokeMethod(this, "findTestCases", Qt::QueuedConnection);
}

void CTestFindJob::findTestCases()
{
    if (!m_suite->arguments().isEmpty())
    {
        KDevelop::ICore::self()->testController()->addTestSuite(m_suite);
        emitResult();
        return;
    }

    m_pendingFiles.clear();
    const auto& sourceFiles = m_suite->sourceFiles();
    for (const auto& file : sourceFiles) {
        if (!file.isEmpty())
        {
            m_pendingFiles << file;
        }
    }
    qCDebug(CMAKE) << "Source files to update:" << m_pendingFiles;

    if (m_pendingFiles.isEmpty())
    {
        KDevelop::ICore::self()->testController()->addTestSuite(m_suite);
        emitResult();
        return;
    }

    const auto currentPendingFiles = m_pendingFiles;
    for (const KDevelop::Path& file : currentPendingFiles) {
        KDevelop::DUChain::self()->updateContextForUrl(KDevelop::IndexedString(file.toUrl()), KDevelop::TopDUContext::AllDeclarationsAndContexts, this);
    }
}

void CTestFindJob::updateReady(const KDevelop::IndexedString& document, const KDevelop::ReferencedTopDUContext& context)
{
    auto* const core = KDevelop::ICore::self();
    if (Q_UNLIKELY(!core || core->shuttingDown())) {
        qCDebug(CMAKE).nospace() << "Cannot add the test suite. KDevelop must be exiting"
                                 << (!core ? " and the KDevelop core already destroyed."
                                           : ".");
        return;
    }

    if (Q_UNLIKELY(!m_suite->project())) {
        qCDebug(CMAKE) << "Cannot add the test suite because its project is already destroyed (probably closed by the user).";
        return;
    }

    qCDebug(CMAKE) << "context update ready" << m_pendingFiles << document.str();
    m_suite->loadDeclarations(document, context);
    m_pendingFiles.removeAll(KDevelop::Path(document.toUrl()));

    if (m_pendingFiles.isEmpty())
    {
        core->testController()->addTestSuite(m_suite);
        emitResult();
    }
}

bool CTestFindJob::doKill()
{
    KDevelop::ICore::self()->languageController()->backgroundParser()->revertAllRequests(this);
    return true;
}
