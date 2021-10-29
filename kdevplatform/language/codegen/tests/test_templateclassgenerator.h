/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TESTTEMPLATECLASSGENERATOR_H
#define KDEVPLATFORM_TESTTEMPLATECLASSGENERATOR_H

#include <QTest>
#include <QTemporaryDir>
#include <QUrl>

#include <memory>

#include "language/codegen/codedescription.h"

namespace KDevelop
{
class TemplateClassGenerator;
}

class TestTemplateClassGenerator : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void fileLabelsCpp();
    void fileLabelsYaml();
    void defaultFileUrlsCpp();
    void defaultFileUrlsYaml();
    void customOptions();
    void templateVariablesCpp();
    void templateVariablesYaml();
    void codeDescription();

    void generate();
    void cppOutput();
    void yamlOutput();

private:
    std::unique_ptr<KDevelop::TemplateClassGenerator> loadTemplate(const QString& name);

private:
    QUrl baseUrl;
    QTemporaryDir tempDir;
    KDevelop::ClassDescription description;
};


#endif // KDEVPLATFORM_TESTTEMPLATECLASSGENERATOR_H
