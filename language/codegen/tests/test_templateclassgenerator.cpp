/*
 * This file is part of KDevelop
 * Copyright 2012 Miha Čančula <miha@noughmad.eu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "test_templateclassgenerator.h"
#include "language/codegen/templateclassgenerator.h"
#include "language/codegen/templatesmodel.h"

#include "tests/autotestshell.h"
#include "tests/testcore.h"

#include <KComponentData>
#include <KStandardDirs>
#include <KUrl>

#include <qtest_kde.h>

using namespace KDevelop;

void TestTemplateClassGenerator::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);

    generator = new TemplateClassGenerator(baseUrl);
    KComponentData data = ICore::self()->componentData();

    // Needed for extracting description out of template archives
    TemplatesModel model(data, this);
    model.setDescriptionResourceType("filetemplate_descriptions");
    model.setTemplateResourceType("filetemplates");
    model.refresh();

    // TODO: Maybe prepare a fixed test template instead of loading a real one
    bool found = false;
    foreach (const QString& description, data.dirs()->findAllResources("filetemplate_descriptions"))
    {
        if (QFileInfo(description).baseName() == "cpp_basic")
        {
            generator->setTemplateDescription(description);
            found = true;
            break;
        }
    }

    QVERIFY(found);

    generator->setIdentifier("ClassName");
}

void TestTemplateClassGenerator::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestTemplateClassGenerator::fileLabels()
{
    QHash<QString,QString> labels = generator->fileLabels();
    QCOMPARE(labels.size(), 2);

    /*
     * File labels can be translated, so we don't check their equality here.
     * But they have to be present and non-empty
     */
    QVERIFY(labels.contains("Header"));
    QVERIFY(!labels["Header"].isEmpty());
    QVERIFY(labels.contains("Implementation"));
    QVERIFY(!labels["Implementation"].isEmpty());
}

void TestTemplateClassGenerator::defaultFileUrls()
{
    QHash<QString,KUrl> files = generator->fileUrlsFromBase(baseUrl, false);
    QCOMPARE(files.size(), 2);

    QVERIFY(files.contains("Header"));
    KUrl expectedHeaderUrl(baseUrl);
    expectedHeaderUrl.addPath("ClassName.h");
    QCOMPARE(files["Header"], expectedHeaderUrl);

    QVERIFY(files.contains("Implementation"));
    KUrl expectedImplementationUrl(baseUrl);
    expectedImplementationUrl.addPath("ClassName.cpp");
    QCOMPARE(files["Implementation"], expectedImplementationUrl);
}

void TestTemplateClassGenerator::lowercaseFileUrls()
{
    QHash<QString,KUrl> files = generator->fileUrlsFromBase(baseUrl, true);
    QCOMPARE(files.size(), 2);

    QVERIFY(files.contains("Header"));
    KUrl expectedHeaderUrl(baseUrl);
    expectedHeaderUrl.addPath("classname.h");
    QCOMPARE(files["Header"], expectedHeaderUrl);

    QVERIFY(files.contains("Implementation"));
    KUrl expectedImplementationUrl(baseUrl);
    expectedImplementationUrl.addPath("classname.cpp");
    QCOMPARE(files["Implementation"], expectedImplementationUrl);
}

QTEST_KDEMAIN(TestTemplateClassGenerator, NoGUI)