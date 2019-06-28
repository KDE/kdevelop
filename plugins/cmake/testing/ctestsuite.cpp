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

Declaration* findTestClassDeclaration(const CursorInRevision& c, DUContext* ctx, RangeInRevision::ContainsBehavior behavior)
{
    /*
     * This code is mostly copied from DUChainUtils::itemUnderCursorInternal.
     * However, it is simplified because we are only looking for uses of the test class,
     * so we can skip the search through local declarations, which speeds up the search.
     * Additionally, we are only interested in the declaration itself, not in its context
     * or range.
     */

    const auto subContexts = ctx->childContexts();
    for (DUContext* subCtx : subContexts) {
        //This is a little hacky, but we need it in case of foreach macros and similar stuff
        if(subCtx->range().contains(c, behavior) || subCtx->range().isEmpty() || subCtx->range().start.line == c.line || subCtx->range().end.line == c.line)
        {
            Declaration *d = findTestClassDeclaration(c, subCtx, behavior);
            if (d)
            {
                return d;
            }
        }
    }

    for(int a = 0; a < ctx->usesCount(); ++a)
    {
        if(ctx->uses()[a].m_range.contains(c, behavior))
        {
            return ctx->topContext()->usedDeclarationForIndex(ctx->uses()[a].m_declarationIndex);
        }
    }

    return nullptr;
}

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

void CTestSuite::loadDeclarations(const IndexedString& document, const KDevelop::ReferencedTopDUContext& ref)
{
    DUChainReadLocker locker(DUChain::lock());
    TopDUContext* topContext = DUChainUtils::contentContextFromProxyContext(ref.data());
    if (!topContext)
    {
        qCDebug(CMAKE) << "No top context in" << document.str();
        return;
    }

    Declaration* testClass = nullptr;

    const auto mainId = Identifier(QStringLiteral("main"));
    const auto mainDeclarations = topContext->findLocalDeclarations(mainId);
    for (Declaration* declaration : mainDeclarations) {
        if (declaration->isDefinition())
        {
            qCDebug(CMAKE) << "Found a definition for a function 'main()' at" << declaration->range();

            /*
             * This is a rather hacky solution to get the test class for a Qt test.
             *
             * The class is used as the argument to the QTEST_MAIN or QTEST_GUILESS_MAIN macro.
             * This macro expands to a main() function with a variable declaration with 'tc' as
             * the name and with the test class as the type.
             *
             * Unfortunately, we cannot get to the function body context in order to find
             * this variable declaration.
             * Instead, we find the cursor to the beginning of the main() function, offset
             * the cursor to the inside of the QTEST_MAIN(x) call, and find the declaration there.
             * If it is a type declaration, that type is the main test class.
             */

            CursorInRevision cursor = declaration->range().start;
            Declaration* testClassDeclaration = nullptr;
            int mainDeclarationColumn = cursor.column;

            // cursor points to the start of QTEST_MAIN(x) invocation, we offset it to point inside it
            cursor.column += 12;
            testClassDeclaration = findTestClassDeclaration(cursor, topContext, RangeInRevision::Default);

            while (!testClassDeclaration || testClassDeclaration->kind() != Declaration::Kind::Type)
            {
                // If the first found declaration was not a type, the macro may be QTEST_GUILESS_MAIN rather than QTEST_MAIN.
                // Alternatively, it may be called as QTEST_MAIN(KDevelop::TestCase), or something similar.
                // So we just try a couple of different positions.
                cursor.column += 8;
                if (cursor.column > mainDeclarationColumn + 60)
                {
                    break;
                }
                testClassDeclaration = findTestClassDeclaration(cursor, topContext, RangeInRevision::Default);
            }

            if (testClassDeclaration && testClassDeclaration->kind() == Declaration::Kind::Type)
            {
                qCDebug(CMAKE) << "Found test class declaration" << testClassDeclaration->identifier().toString() << testClassDeclaration->kind();
                if (StructureType::Ptr type = testClassDeclaration->type<StructureType>())
                {
                    testClass = type->declaration(topContext);
                    if (testClass && testClass->internalContext())
                    {
                        break;
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

    const auto testClassDeclarations = testClass->internalContext()->localDeclarations(topContext);
    for (Declaration* decl : testClassDeclarations) {
        qCDebug(CMAKE) << "Found declaration" << decl->toString() << decl->identifier().identifier().byteArray();
        if (auto* function = dynamic_cast<ClassFunctionDeclaration*>(decl))
        {
            if (function->accessPolicy() == Declaration::Private && function->isSlot())
            {
                QString name = function->qualifiedIdentifier().last().toString();
                qCDebug(CMAKE) << "Found private slot in test" << name;

                if (name.endsWith(QLatin1String("_data")))
                {
                    continue;
                }

                if (name != QLatin1String("initTestCase") && name != QLatin1String("cleanupTestCase")
                    && name != QLatin1String("init") && name != QLatin1String("cleanup"))
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
