/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEST_TEMPLATERENDERER_H
#define KDEVPLATFORM_TEST_TEMPLATERENDERER_H

#include <QTest>
#include <QObject>

namespace KDevelop
{
class TemplateRenderer;
}

class TestTemplateRenderer : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void simpleVariables_data();
    void simpleVariables();

    void includeTemplates();
    void kdevFilters();
    void kdevFiltersWithLookup();

private:
    KDevelop::TemplateRenderer* renderer;

};

#endif // KDEVPLATFORM_TEST_TEMPLATERENDERER_H
