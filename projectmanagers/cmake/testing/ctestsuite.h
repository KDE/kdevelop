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

#ifndef CTESTSUITE_H
#define CTESTSUITE_H

#include <interfaces/itestsuite.h>
#include <language/duchain/indexeddeclaration.h>
#include <KUrl>

namespace KDevelop {
class ITestController;
class ReferencedTopDUContext;
}

class CTestSuite : public KDevelop::ITestSuite
{
public:
    CTestSuite(const QString& name, const KUrl& executable, const QStringList& files, KDevelop::IProject* project, const QStringList& args = QStringList());
    virtual ~CTestSuite();
    
    virtual KJob* launchCase(const QString& testCase);
    virtual KJob* launchCases(const QStringList& testCases);
    virtual KJob* launchAllCases();
    
    virtual KUrl executable() const;
    virtual QStringList cases() const;
    virtual QString name() const;
    virtual KDevelop::IProject* project() const;
    virtual KDevelop::TestResult result() const;
    
    virtual KDevelop::IndexedDeclaration declaration() const;
    virtual KDevelop::IndexedDeclaration caseDeclaration(const QString& testCase) const;
    
    QStringList arguments() const;
    void setResult(const KDevelop::TestResult& result);
    void setTestCases(QStringList cases);
    QStringList sourceFiles() const;
    void loadDeclarations(const KDevelop::IndexedString& document, const KDevelop::ReferencedTopDUContext& context);
    
private:
    KUrl m_executable;
    QString m_name;
    QStringList m_cases;
    QStringList m_args;
    QStringList m_files;
    KDevelop::IProject* m_project;
    
    QHash<QString, KDevelop::IndexedDeclaration> m_declarations;
    KDevelop::IndexedDeclaration m_suiteDeclaration;
    KDevelop::TestResult m_result;
};

#endif // CTESTSUITE_H
