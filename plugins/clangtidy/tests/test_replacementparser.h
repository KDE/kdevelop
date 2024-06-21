/*
    SPDX-FileCopyrightText: 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REPLACEMENTSPARSERTESTER_H
#define REPLACEMENTSPARSERTESTER_H

#include <QObject>
#include <QString>

class ReplacementsParserTester : public QObject
{
    Q_OBJECT
public:
    struct TestFilePaths
    {
        QString yaml;
        QString source;
    };

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void doTest();

private:
    TestFilePaths m_pluginFilePaths;
    TestFilePaths m_nihonPluginFilePaths;
};

#endif // REPLACEMENTSPARSERTESTER_H
