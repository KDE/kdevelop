/*
 * This file is part of KDevelop
 * Copyright 2012 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "sharedurl.h"

#include <QtTest>

#include <KUrl>

QTEST_MAIN(SharedUrl);

static const int FILES_PER_FOLDER = 10;
static const int FOLDERS_PER_FOLDER = 5;
static const int TREE_DEPTH = 5;

struct OptimizedUrl
{
    OptimizedUrl()
    { }
    OptimizedUrl(const QString& path)
    : m_data(path.split('/', QString::SkipEmptyParts).toVector())
    {
        Q_ASSERT(path.startsWith('/'));
        Q_ASSERT(!path.contains("/../"));
    }
    OptimizedUrl(const OptimizedUrl& other, const QString& child = QString())
    : m_data(other.m_data)
    {
        if (!child.isEmpty()) {
            Q_ASSERT(!child.startsWith('/'));
            Q_ASSERT(!child.contains("../"));
            m_data += child.split('/', QString::SkipEmptyParts).toVector();
        }
    }
    bool operator==(const OptimizedUrl& other) const
    {
        return m_data == other.m_data;
    }

private:
    QVector<QString> m_data;
};

template<typename T>
T stringToUrl(const QString& path)
{
    return T(path);
}

template<>
QStringList stringToUrl(const QString& path)
{
    return path.split('/');
}

template<typename T>
T childUrl(const T& parent, const QString& child)
{
    return T(parent, child);
}

template<>
QStringList childUrl(const QStringList& parent, const QString& child)
{
    QStringList ret = parent;
    ret << child;
    return ret;
}

template<>
QUrl childUrl(const QUrl& parent, const QString& child)
{
    QUrl ret = parent;
    ret.setPath(ret.path() + '/' + child);
    return ret;
}

template<>
KUrl childUrl(const KUrl& parent, const QString& child)
{
    KUrl ret = parent;
    ret.addPath(child);
    return ret;
}

template<typename T>
QVector<T> generateData(const T& parent, int level)
{
    QVector<T> ret;
    // files per folder
    for (int i = 0; i < FILES_PER_FOLDER; ++i) {
        const QString fileName = QString("file%1.txt").arg(i);
        const T file = childUrl<T>(parent, fileName);
        Q_ASSERT(!ret.contains(file));
        ret << file;
    }
    // nesting depth
    if (level < TREE_DEPTH) {
        // folders per folder
        for (int i = 0; i < FOLDERS_PER_FOLDER; ++i) {
            const QString folderName = QString("folder%1").arg(i);
            const T folder = childUrl<T>(parent, folderName);
            Q_ASSERT(!ret.contains(folder));
            ret << folder;
            ret += generateData<T>(folder, level + 1);
        }
    }
    return ret;
}

template<typename T>
QVector<T> generateData()
{
    const T base = stringToUrl<T>("/tmp/foo/bar");
    QVector<T> ret;
    ret << base;
    ret += generateData(base, 0);
    return ret;
}

void SharedUrl::kurl()
{
    QVector<KUrl> urls = generateData<KUrl>();
}

void SharedUrl::qurl()
{
    QVector<QUrl> urls = generateData<QUrl>();
}

void SharedUrl::qstringlist()
{
    QVector<QStringList> urls = generateData<QStringList>();
}

void SharedUrl::optimized()
{
    QVector<OptimizedUrl> urls = generateData<OptimizedUrl>();
}


#include "sharedurl.moc"
