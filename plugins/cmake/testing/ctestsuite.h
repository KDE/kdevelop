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
#include <util/path.h>
#include <QHash>
#include <QPointer>

namespace KDevelop {
class ReferencedTopDUContext;
}

class CTestSuite : public KDevelop::ITestSuite
{
public:
    CTestSuite(const QString& name, const KDevelop::Path &executable, const QList<KDevelop::Path>& files, KDevelop::IProject* project, const QStringList& args, const QHash<QString, QString>& properties);
    ~CTestSuite() override;

    KJob* launchCase(const QString& testCase, TestJobVerbosity verbosity) override;
    KJob* launchCases(const QStringList& testCases, TestJobVerbosity verbosity) override;
    KJob* launchAllCases(TestJobVerbosity verbosity) override;

    virtual KDevelop::Path executable() const;
    QStringList cases() const override;
    QString name() const override;
    KDevelop::IProject* project() const override;

    KDevelop::IndexedDeclaration declaration() const override;
    KDevelop::IndexedDeclaration caseDeclaration(const QString& testCase) const override;

    virtual QHash<QString, QString> properties() const;

    QStringList arguments() const;
    void setTestCases(const QStringList& cases);
    QList<KDevelop::Path> sourceFiles() const;
    void loadDeclarations(const KDevelop::IndexedString& document, const KDevelop::ReferencedTopDUContext& context);

private:
    KDevelop::Path m_executable;
    QString m_name;
    QStringList m_cases;
    QStringList m_args;
    QList<KDevelop::Path> m_files;
    QPointer<KDevelop::IProject> m_project;

    QHash<QString, KDevelop::IndexedDeclaration> m_declarations;
    QHash<QString, QString> m_properties;
    KDevelop::IndexedDeclaration m_suiteDeclaration;
};

#endif // CTESTSUITE_H
