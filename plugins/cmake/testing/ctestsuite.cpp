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
#include <debug.h>

#include <interfaces/itestcontroller.h>
#include <interfaces/iproject.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/declaration.h>
#include <language/duchain/indexeddeclaration.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/functiondeclaration.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/pointertype.h>
#include <language/duchain/types/referencetype.h>
#include <language/duchain/types/structuretype.h>

using namespace KDevelop;

CTestSuite::CTestSuite(const QString& name, const KDevelop::Path &executable, const QList<KDevelop::Path>& files, IProject* project, const QStringList& args, const QHash<QString, QString>& properties):
m_executable(executable),
m_name(name),
m_args(args),
m_files(files),
m_project(project),
m_properties(properties)
{
    Q_ASSERT(project);
    qCDebug(CMAKE) << m_name << m_executable << m_project->name();
}

CTestSuite::~CTestSuite()
{

}

bool CTestSuite::findCaseDeclarations(const QVector<Declaration*> &classDeclarations)
{
    for (Declaration* decl : classDeclarations) {
        qCDebug(CMAKE) << "Found declaration" << decl->toString() << decl->identifier().identifier().byteArray();

        const auto* const function = dynamic_cast<ClassFunctionDeclaration*>(decl);
        if (!function || !(function->accessPolicy() == Declaration::Private && function->isSlot())) {
            continue;
        }
        QString name = function->qualifiedIdentifier().last().toString();
        qCDebug(CMAKE) << "Found private slot in test" << name;

        if (name.endsWith(QLatin1String("_data"))) {
            continue;
        }

        const auto functionType = function->type<FunctionType>();
        if (!functionType || functionType->indexedArgumentsSize() > 0) {
            // function declarations with arguments are not valid test functions
            continue;
        }
        qCDebug(CMAKE) << "Found test case function declaration" << function->identifier().toString();

        if (name != QLatin1String("initTestCase") && name != QLatin1String("cleanupTestCase") &&
            name != QLatin1String("init") && name != QLatin1String("cleanup"))
        {
            m_cases << name;
        } else {
            continue;
        }

        const auto* def = FunctionDefinition::definition(decl);
        m_declarations[name] = def ? IndexedDeclaration(def) : IndexedDeclaration(function);
    }
    return !m_declarations.isEmpty();
}

void CTestSuite::loadDeclarations(const IndexedString& document, const KDevelop::ReferencedTopDUContext& ref)
{
    DUChainReadLocker locker(DUChain::lock());
    TopDUContext* topContext = DUChainUtils::contentContextFromProxyContext(ref.data());
    if (!topContext) {
        qCDebug(CMAKE) << "No top context in" << document.str();
        return;
    }

    Declaration* testClass = nullptr;
    const auto mainId = Identifier(QStringLiteral("main"));
    const auto mainDeclarations = topContext->findLocalDeclarations(mainId);
    DUContext* tmpInternalContext;
    for (Declaration* declaration : mainDeclarations) {
        if (!declaration->isDefinition() || !(tmpInternalContext = declaration->internalContext())) {
            continue;
        }
        RangeInRevision contextRange = tmpInternalContext->range();
        qCDebug(CMAKE) << "Found a definition for a function 'main()' at" << contextRange;

        /*
            * This function tries to deduce the test class from the main function definition of
            * the test source file. To do so, the cursor is set before the end of the internal
            * context. Going backwards from there, the first variable declaration of a class
            * type with private slots is assumed to be the test class.
            * This method finds test classes passed to QTEST_MAIN or QTEST_GUILESS_MAIN, but
            * also some classes which are used in manual implementations with a similar structure
            * as in the Qt macros.
            * If no such class is found, the main function definition is linked to the test suite
            * and no test cases are added to the test suite.
        */

        --contextRange.end.column; // set cursor before the end of the definition
        const auto innerContext = topContext->findContextAt(contextRange.end, true);
        if (!innerContext) {
            continue;
        }
        const auto mainDeclarations = innerContext->localDeclarations(topContext);
        for (auto it = mainDeclarations.rbegin(); it != mainDeclarations.rend(); ++it) {
            qCDebug(CMAKE) << "Main declaration" << (*it)->toString();

            auto type = (*it)->abstractType();
            // Strip pointer and reference types to finally get to the structure type of the test class
            while (type && (type->whichType() == AbstractType::TypePointer || type->whichType() == AbstractType::TypeReference)) {
                if (const auto ptype = TypePtr<PointerType>::dynamicCast(type)) {
                    type = ptype->baseType();
                } else if (const auto rtype = TypePtr<ReferenceType>::dynamicCast(type)) {
                    type = rtype->baseType();
                } else {
                    type = nullptr;
                }
            }
            const auto structureType = TypePtr<StructureType>::dynamicCast(type);
            if (!structureType) {
                continue;
            }

            testClass = structureType->declaration(topContext);
            if (!testClass || !(tmpInternalContext = testClass->internalContext())) {
                continue;
            }

            if (findCaseDeclarations(tmpInternalContext->localDeclarations(topContext))) {
                break;
            }
        }
        testClass = declaration;
    }

    if (testClass && testClass->internalContext()) {
        m_suiteDeclaration = IndexedDeclaration(testClass);
    } else {
        qCDebug(CMAKE) << "No test class found or internal context missing in " << document.str();
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
    return new CTestRunJob(this, testCases, outputVerbosity);
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
    return m_declarations.value(testCase, IndexedDeclaration(nullptr));
}

void CTestSuite::setTestCases(const QStringList& cases)
{
    m_cases = cases;
}

QList<KDevelop::Path> CTestSuite::sourceFiles() const
{
    return m_files;
}

QHash<QString, QString> CTestSuite::properties() const
{
    return m_properties;
}
