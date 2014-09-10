
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

#ifndef KDEVPLATFORM_TESTTEMPLATECLASSGENERATOR_H
#define KDEVPLATFORM_TESTTEMPLATECLASSGENERATOR_H

#include <QtTest>
#include <QTemporaryDir>
#include <QUrl>

#include "language/codegen/codedescription.h"

namespace KDevelop
{
class TemplateClassGenerator;
}

class TestTemplateClassGenerator : public QObject
{
    Q_OBJECT

private slots:
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
    KDevelop::TemplateClassGenerator* loadTemplate(const QString& name);

private:
    QUrl baseUrl;
    QTemporaryDir tempDir;
    KDevelop::ClassDescription description;
    void setLowercaseFileNames(KDevelop::TemplateClassGenerator* generator);
};


#endif // KDEVPLATFORM_TESTTEMPLATECLASSGENERATOR_H
