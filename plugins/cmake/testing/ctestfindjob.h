/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CTESTFINDJOB_H
#define CTESTFINDJOB_H

#include <KJob>
#include <util/path.h>

namespace KDevelop {
class IndexedString;
class ReferencedTopDUContext;
}

class CTestSuite;

class CTestFindJob : public KJob
{
    Q_OBJECT
    
public:
    explicit CTestFindJob(CTestSuite* suite, QObject* parent = nullptr);
    void start() override;
    
private Q_SLOTS:
    void findTestCases();
    void updateReady(const KDevelop::IndexedString& document, const KDevelop::ReferencedTopDUContext& context);

protected:
    bool doKill() override;
private:
    CTestSuite* m_suite;
    QList<KDevelop::Path> m_pendingFiles;
};

#endif // CTESTFINDJOB_H
