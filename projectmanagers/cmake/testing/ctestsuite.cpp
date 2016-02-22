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

#include "ctestsuite.h"
#include "ctestrunjob.h"
#include "../debug.h"

#include <QFileInfo>

#include <interfaces/itestcontroller.h>
#include <interfaces/iproject.h>
#include <language/duchain/indexeddeclaration.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/use.h>
#include <language/duchain/declaration.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/functiondeclaration.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/types/structuretype.h>
#include <project/projectmodel.h>


using namespace KDevelop;

CTestSuite::CTestSuite(const QString& name, const KDevelop::Path &executable, const QList<KDevelop::Path>& files, IProject* project, const QStringList& args, bool expectFail):
m_executable(executable),
m_name(name),
m_args(args),
m_files(files),
m_project(project),
m_expectFail(expectFail)
{
    Q_ASSERT(project);
    qCDebug(CMAKE) << m_name << m_executable << m_project->name();
}

CTestSuite::~CTestSuite()
{

}

void CTestSuite::loadDeclarations(const IndexedString& document, const KDevelop::ReferencedTopDUContext& ref)
{
    DUChainReadLocker locker(DUChain::lock());
    TopDUContext* topContext = DUChainUtils::contentContextFromProxyContext(ref.data());
    if (!topContext)
    {
        qCDebug(CMAKE) << "No top context in" << document.str();
        return;
    }

    Declaration* testClass = 0;
    Identifier testCaseIdentifier("tc");
    foreach (Declaration* declaration, topContext->findLocalDeclarations(Identifier("main")))
    {
        if (declaration->isDefinition())
        {
            qCDebug(CMAKE) << "Found a definition for a function 'main()' ";
            FunctionDefinition* def = dynamic_cast<FunctionDefinition*>(declaration);
            DUContext* main = def->internalContext();
            foreach (Declaration* mainDeclaration, main->localDeclarations(topContext))
            {
                if (mainDeclaration->identifier() == testCaseIdentifier)
                {
                    qCDebug(CMAKE) << "Found tc declaration in main:" << mainDeclaration->identifier().toString();
                    qCDebug(CMAKE) << "Its type is" << mainDeclaration->abstractType()->toString();
                    if (StructureType::Ptr type = mainDeclaration->abstractType().cast<StructureType>())
                    {
                        testClass = type->declaration(topContext);
                    }
                }
            }
        }
    }

    if (!testClass || !testClass->internalContext())
    {
        qCDebug(CMAKE) << "No test class found or internal context missing in " << document.str();
        return;
    }

    if (!m_suiteDeclaration.data())
    {
        m_suiteDeclaration = IndexedDeclaration(testClass);
    }

    foreach (Declaration* decl, testClass->internalContext()->localDeclarations(topContext))
    {
        qCDebug(CMAKE) << "Found declaration" << decl->toString() << decl->identifier().identifier().byteArray();
        if (ClassFunctionDeclaration* function = dynamic_cast<ClassFunctionDeclaration*>(decl))
        {
            if (function->accessPolicy() == Declaration::Private && function->isSlot())
            {
                QString name = function->qualifiedIdentifier().last().toString();
                qCDebug(CMAKE) << "Found private slot in test" << name;

                if (name.endsWith("_data"))
                {
                    continue;
                }

                if (name != "initTestCase" && name != "cleanupTestCase"
                    && name != "init" && name != "cleanup")
                {
                    m_cases << name;
                }
                qCDebug(CMAKE) << "Found test case function declaration" << function->identifier().toString();

                FunctionDefinition* def = FunctionDefinition::definition(decl);
                m_declarations[name] = def ? IndexedDeclaration(def) : IndexedDeclaration(function);
            }
        }
    }
}

KJob* CTestSuite::launchCase(const QString& testCase, TestJobVerbosity verbosity)
{
    return launchCases(QStringList() << testCase, verbosity);
}

KJob* CTestSuite::launchCases(const QStringList& testCases, ITestSuite::TestJobVerbosity verbosity)
{
    qCDebug(CMAKE) << "Launching test run" << m_name << "with cases" << testCases;

    OutputJob::OutputJobVerbosity outputVerbosity = (verbosity == Verbose) ? OutputJob::Verbose : OutputJob::Silent;
    return new CTestRunJob(this, testCases, outputVerbosity, m_expectFail);
}

KJob* CTestSuite::launchAllCases(TestJobVerbosity verbosity)
{
    return launchCases(cases(), verbosity);
}

KDevelop::Path CTestSuite::executable() const
{
    return m_executable;
}

QStringList CTestSuite::cases() const
{
    return m_cases;
}

QString CTestSuite::name() const
{
    return m_name;
}

KDevelop::IProject* CTestSuite::project() const
{
    return m_project;
}

QStringList CTestSuite::arguments() const
{
    return m_args;
}

IndexedDeclaration CTestSuite::declaration() const
{
    return m_suiteDeclaration;
}

IndexedDeclaration CTestSuite::caseDeclaration(const QString& testCase) const
{
    return m_declarations.value(testCase, IndexedDeclaration(0));
}

void CTestSuite::setTestCases(const QStringList& cases)
{
    m_cases = cases;
}

QList<KDevelop::Path> CTestSuite::sourceFiles() const
{
    return m_files;
}



