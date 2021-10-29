/*
    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef TEST_ASTYLE_H
#define TEST_ASTYLE_H

#include <QObject>

#include <memory>

class AStyleFormatter;

class TestAstyle : public QObject
{
    Q_OBJECT
public:
    ~TestAstyle();

private Q_SLOTS:
    void initTestCase();

    void testFuzzyMatching();
    void testTabMatching();
    void renameVariable();
    void overrideHelper();
    void varTypeAssistant();
    void testMultipleFormatters();
    void testMacroFormatting();
    void testContext();
    void testTabIndentation();
    void testForeach();
    void testPointerAlignment();
    void testKdeFrameworks();
    void testKdeFrameworks_data();

private:
    std::unique_ptr<AStyleFormatter> m_formatter;
};

#endif // TEST_ASTYLE_H
