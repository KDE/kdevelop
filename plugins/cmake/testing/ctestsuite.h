/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-or-later
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
class Declaration;
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
    bool findCaseDeclarations(const QVector<KDevelop::Declaration*> &classDeclarations);

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
