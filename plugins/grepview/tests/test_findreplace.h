/*
    SPDX-FileCopyrightText: 1999-2001 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 1999-2001 the KDevelop Team
    SPDX-FileCopyrightText: 2010 Julien Desgats <julien.desgats@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_REPLACETEST_H
#define KDEVPLATFORM_PLUGIN_REPLACETEST_H

#include <QObject>
#include <QList>
#include <QMetaType>
#include <QPair>

struct GrepJobSettings;

namespace KDevelop
{
class TestProjectController;
}
class GrepViewPlugin;

class FindReplaceTest : public QObject
{
    Q_OBJECT
public:
    struct Match {
        Match() {}
        Match(int l,int s,int e) : line(l), start(s), end(e) {}
        int line;
        int start;
        int end;
    };
    using MatchList = QList<Match>;

    using File = QPair<QString, QString>;  /// Represent a file with name => content
    using FileList = QList<File>;

private:
    GrepViewPlugin* m_plugin;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();

    void testFind();
    void testFind_data();

    void testSingleFileAsDirectoryChoice();

    void testIncludeExcludeFilters();
    void testIncludeExcludeFilters_data();

    void testReplace();
    void testReplace_data();

private:
    void addTestProjectFromFileSystem(const QString& path);
    template<typename Test>
    void varyProjectFilesOnly(GrepJobSettings& settings, const QString& projectPath, Test testToRun);

    KDevelop::TestProjectController* m_projectController;
};

Q_DECLARE_METATYPE(FindReplaceTest::MatchList)
Q_DECLARE_METATYPE(FindReplaceTest::FileList)

#endif // KDEVPLATFORM_PLUGIN_REPLACETEST_H
