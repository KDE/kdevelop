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

#include <KProcess>
#include <KDebug>
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
#include <project/projectmodel.h>


using namespace KDevelop;

CTestSuite::CTestSuite(const QString& name, const KUrl& executable, const QStringList& files, IProject* project, const QStringList& args): 
m_executable(executable),
m_name(name),
m_args(args),
m_project(project)
{
    m_executable.cleanPath();
    Q_ASSERT(project);
    kDebug() << m_name << m_executable << m_project->name();
    
    foreach (const QString& file, files)
    {
        KUrl url(file);
        url.cleanPath();
        m_files << url.toLocalFile();
    }
}

CTestSuite::~CTestSuite()
{

}

void CTestSuite::loadDeclarations(const IndexedString& document, const KDevelop::ReferencedTopDUContext& ref)
{
    DUChainReadLocker locker(DUChain::lock());
    TopDUContext* context = DUChainUtils::contentContextFromProxyContext(ref.data());
    if (!context)
    {
        kDebug() << "No top context in" << document.str();
        return;
    }
    
    kDebug() << "Found" << context->localDeclarations(context).size() << "declarations in file" << document.str();
    foreach (Declaration* decl, context->localDeclarations(context))
    {
        kDebug() << "Found declaration" << decl->toString() << decl->identifier().identifier().byteArray();
        FunctionDefinition* def = 0;
        if (decl->isDefinition() && (def = dynamic_cast<FunctionDefinition*>(decl)))
        {
            decl = def->declaration(context);
        }
        
        if (ClassFunctionDeclaration* function = dynamic_cast<ClassFunctionDeclaration*>(decl))
        {
            if (function->accessPolicy() == Declaration::Private && function->isSlot())
            {
                QString name = function->qualifiedIdentifier().last().toString();
                kDebug() << "Found private slot in test" << name; 
                
                if (!m_suiteDeclaration.data())
                {
                    m_suiteDeclaration = IndexedDeclaration(function->context()->owner());
                }
                
                if (name != "initTestCase" && name != "cleanupTestCase")
                {
                    m_cases << name;
                }
                kDebug() << "Found test case function declaration" << function->identifier().toString();
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
    kDebug() << "Launching test run" << m_name << "with cases" << testCases;
    
    OutputJob::OutputJobVerbosity outputVerbosity = (verbosity == Verbose) ? OutputJob::Verbose : OutputJob::Silent;
    return new CTestRunJob(this, testCases, outputVerbosity);
}

KJob* CTestSuite::launchAllCases(TestJobVerbosity verbosity)
{
    return launchCases(cases(), verbosity);
}

KUrl CTestSuite::executable() const
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

void CTestSuite::setTestCases(QStringList cases)
{
    m_cases = cases;
}

QStringList CTestSuite::sourceFiles() const
{
    return m_files;
}



