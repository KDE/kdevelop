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

#ifndef CTESTFINDJOB_H
#define CTESTFINDJOB_H

#include <KJob>
#include <util/path.h>

#include <memory>

namespace KDevelop {
class IndexedString;
class ReferencedTopDUContext;
}

class CTestSuite;

class CTestFindJob : public KJob
{
    Q_OBJECT
    
public:
    explicit CTestFindJob(std::unique_ptr<CTestSuite> suite, QObject* parent = nullptr);
    ~CTestFindJob() override;
    void start() override;
    
private Q_SLOTS:
    void findTestCases();
    void updateReady(const KDevelop::IndexedString& document, const KDevelop::ReferencedTopDUContext& context);

protected:
    bool doKill() override;
private:
    std::unique_ptr<CTestSuite> m_suite;
    QList<KDevelop::Path> m_pendingFiles;
};

#endif // CTESTFINDJOB_H
