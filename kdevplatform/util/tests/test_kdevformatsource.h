/*
    SPDX-FileCopyrightText: 2016 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QScopedPointer>
#include <QVector>

class QTemporaryDir;

namespace KDevelop {

struct Source
{
    QString path;
    QStringList lines;
};

class TestKdevFormatSource : public QObject
{
    Q_OBJECT

public:
    TestKdevFormatSource();
    ~TestKdevFormatSource() override;

private Q_SLOTS:
    void initTestCase();

    void testNotFound();
    void testNotFound_data();

    void testNoCommands();
    void testNoCommands_data();

    void testNotMatch();
    void testNotMatch_data();

    void testMatch1();
    void testMatch1_data();

    void testMatch2();
    void testMatch2_data();

    void testWildcardPathMatching();
    void testWildcardPathMatching_data();

private:
    bool initTest(const QStringList& formatFileData);
    void runTest() const;

    bool mkPath(const QString& path) const;
    bool writeLines(const QString& path, const QStringList& lines) const;
    bool readLines(const QString& path, QStringList& lines) const;

    QScopedPointer<QTemporaryDir> m_temporaryDir;
    QVector<Source> m_sources;
};

}
