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

#include "test_templatesmodel.h"
#include "codegen_tests_config.h"

#include <language/codegen/templatesmodel.h>
#include <tests/testcore.h>
#include <tests/autotestshell.h>

#include <KComponentData>
#include <KStandardDirs>

using namespace KDevelop;

void TestTemplatesModel::initTestCase()
{
    AutoTestShell::init();
    TestCore* core = TestCore::initialize(Core::NoUi);

    bool addedDir = core->componentData().dirs()->addResourceDir("data", CODEGEN_TESTS_DATA_DIR, true);
    QVERIFY(addedDir);

    model = new TemplatesModel("kdevcodegentest", this);
    model->refresh();
}

void TestTemplatesModel::cleanupTestCase()
{
    delete model;
    TestCore::shutdown();
}

void TestTemplatesModel::descriptionExtraction()
{
    QCOMPARE(model->rowCount(), 1);
    QModelIndex testingCategoryIndex = model->index(0, 0);
    QCOMPARE(model->rowCount(testingCategoryIndex), 2);

    for (int i = 0; i < 2; ++i)
    {
        QModelIndex languageCategoryIndex = model->index(i, 0, testingCategoryIndex);
        QCOMPARE(model->rowCount(languageCategoryIndex), 1);
        QModelIndex templateIndex = model->index(0, 0, languageCategoryIndex);
        QCOMPARE(model->rowCount(templateIndex), 0);
    }
}

void TestTemplatesModel::descriptionParsing()
{
    QList<QStandardItem*> items = model->findItems("Testing YAML template", Qt::MatchRecursive);
    QCOMPARE(items.size(), 1);
    QStandardItem* item = items.first();

    QCOMPARE(item->data(TemplatesModel::CommentRole).toString(), QString("Describes a class using YAML syntax"));
    QVERIFY(item->data(TemplatesModel::IconNameRole).toString().isEmpty());

    QString descriptionFile = ICore::self()->componentData().dirs()->findResource("data", "kdevcodegentest/template_descriptions/test_yaml.desktop");
    QVERIFY(QFile::exists(descriptionFile));
    QCOMPARE(item->data(TemplatesModel::DescriptionFileRole).toString(), descriptionFile);
}

void TestTemplatesModel::templateIndexes()
{
    QModelIndexList indexes = model->templateIndexes("test_yaml.tar.bz2");
    QCOMPARE(indexes.size(), 3);

    QCOMPARE(model->data(indexes[0]).toString(), QString("Testing"));
    QCOMPARE(model->data(indexes[1]).toString(), QString("YAML"));
    QCOMPARE(model->data(indexes[2]).toString(), QString("Testing YAML template"));
}


QTEST_GUILESS_MAIN(TestTemplatesModel)
