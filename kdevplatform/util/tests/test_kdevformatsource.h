/*
    Copyright 2016 Anton Anikin <anton.anikin@htower.ru>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#pragma once

#include <QObject>
#include <QScopedPointer>
#include <QVector>

class QTemporaryDir;

namespace KDevelop
{

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
