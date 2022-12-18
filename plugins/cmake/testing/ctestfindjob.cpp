/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ctestfindjob.h"
#include "ctestsuite.h"
#include <debug_testing.h>

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/duchain/duchain.h>
#include <language/backgroundparser/backgroundparser.h>

#include <KLocalizedString>

CTestFindJob::CTestFindJob(CTestSuite* suite, QObject* parent)
: KJob(parent)
, m_suite(suite)
{
    qCDebug(CMAKE_TESTING) << "Created a CTestFindJob";
    setObjectName(i18n("Parse test suite %1", suite->name()));
    setCapabilities(Killable);
}

void CTestFindJob::start()
{
    qCDebug(CMAKE_TESTING) << "Finding test cases";
    QMetaObject::invokeMethod(this, &CTestFindJob::findTestCases, Qt::QueuedConnection);
}

void CTestFindJob::findTestCases()
{
    if (!m_suite)
        return;

    m_pendingFiles.clear();
    const auto& sourceFiles = m_suite->sourceFiles();
    for (const auto& file : sourceFiles) {
        if (!file.isEmpty())
        {
            m_pendingFiles << file;
        }
    }
    qCDebug(CMAKE_TESTING) << "Source files to update:" << m_pendingFiles;

    if (m_pendingFiles.isEmpty()) {
        m_suite = nullptr;
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
    if (!m_suite) {
        // killed or done already
        return;
    }

    if (Q_UNLIKELY(!m_suite->project())) {
        qCDebug(CMAKE_TESTING) << "Cannot add test suite" << m_suite->name()
                               << "because its project is already destroyed (probably closed by the user).";
        kill();
        return;
    }

    qCDebug(CMAKE_TESTING) << "context update ready" << m_pendingFiles << document.str();
    m_suite->loadDeclarations(document, context);
    m_pendingFiles.removeAll(KDevelop::Path(document.toUrl()));

    if (m_pendingFiles.isEmpty()) {
        m_suite = nullptr;
        emitResult();
    }
}

bool CTestFindJob::doKill()
{
    m_suite = nullptr;
    KDevelop::ICore::self()->languageController()->backgroundParser()->revertAllRequests(this);
    return true;
}
