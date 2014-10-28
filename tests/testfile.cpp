/* This file is part of KDevelop

   Copyright 2010 Niko Sams <niko.sams@gmail.com>
   Copyright 2011 Milian Wolff <mail@milianw.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "testfile.h"

#include "testproject.h"

#include <QTemporaryFile>
#include <QTime>
#include <QTest>

#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/backgroundparser/backgroundparser.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <project/projectmodel.h>

using namespace KDevelop;

struct TestFile::TestFilePrivate
{
    TestFilePrivate()
    : ready(false)
    , keepDUChainData(false)
    {
    }

    void updateReady(const IndexedString& _url, ReferencedTopDUContext _topContext)
    {
        Q_ASSERT(_url == url);
        Q_UNUSED(_url);
        topContext = _topContext;
        ready = true;
    }

    void init(const QString& fileName, const QString& contents, TestProject* _project)
    {
        file = fileName;

        setFileContents(contents);

        QFileInfo info(file);
        Q_ASSERT(info.exists());
        Q_ASSERT(info.isFile());
        url = IndexedString(info.absoluteFilePath());

        project = _project;
        if (project) {
            fileItem.reset(new ProjectFileItem(_project, Path(file), _project->projectItem()));
        }
    }

    void setFileContents(const QString& contents)
    {
        QFile file(this->file);
        file.open(QIODevice::WriteOnly | QIODevice::Truncate);
        Q_ASSERT(file.isOpen());
        Q_ASSERT(file.isWritable());
        file.write(contents.toUtf8());
        ready = false;
    }

    QString file;
    QString suffix;
    bool ready;
    ReferencedTopDUContext topContext;
    IndexedString url;
    TestProject* project;
    QScopedPointer<ProjectFileItem> fileItem;
    bool keepDUChainData;
};

TestFile::TestFile(const QString& contents, const QString& fileExtension,
                   TestProject* project, const QString& dir)
: d(new TestFilePrivate())
{
    d->suffix = '.' + fileExtension;

    QTemporaryFile file((!dir.isEmpty() ? dir : QDir::tempPath()) + "/testfile_XXXXXX" + d->suffix);
    file.setAutoRemove(false);
    file.open();
    Q_ASSERT(file.isOpen());

    d->init(file.fileName(), contents, project);
}

TestFile::TestFile(const QString& contents, const QString& fileExtension, const TestFile* base)
: d(new TestFilePrivate)
{
    QString fileName = base->d->file.mid(0, base->d->file.length() - base->d->suffix.length());
    d->suffix = '.' + fileExtension;
    fileName += d->suffix;
    d->init(fileName, contents, base->d->project);
}

TestFile::~TestFile()
{
    if (d->topContext && !d->keepDUChainData) {
        DUChainWriteLocker lock;
        DUChain::self()->removeDocumentChain(d->topContext.data());
    }
    QFile::remove(d->file);
    delete d;
}

IndexedString TestFile::url() const
{
    return d->url;
}

void TestFile::parse(TopDUContext::Features features, int priority)
{
    d->ready = false;
    DUChain::self()->updateContextForUrl(d->url, features, this, priority);
}

bool TestFile::parseAndWait(TopDUContext::Features features, int priority, int timeout)
{
    parse(features, priority);
    return waitForParsed(timeout);
}

bool TestFile::waitForParsed(int timeout)
{
    if (!d->ready) {
        // optimize: we don't want to wait the usual timeout before parsing documents here
        ICore::self()->languageController()->backgroundParser()->parseDocuments();
    }
    QTime t;
    t.start();
    while (!d->ready && t.elapsed() < timeout) {
        QTest::qWait(10);
    }
    return d->ready;
}

bool TestFile::isReady() const
{
    return d->ready;
}

ReferencedTopDUContext TestFile::topContext()
{
    waitForParsed();
    return d->topContext;
}

void TestFile::setFileContents(const QString& contents)
{
    d->setFileContents(contents);
}

QString TestFile::fileContents() const
{
    QFile file(d->file);
    file.open(QIODevice::ReadOnly);
    Q_ASSERT(file.isOpen());
    Q_ASSERT(file.isReadable());
    return QString::fromUtf8(file.readAll());
}

void TestFile::setKeepDUChainData(bool keep)
{
    d->keepDUChainData = keep;
}

bool TestFile::keepDUChainData()
{
    return d->keepDUChainData;
}

#include "moc_testfile.cpp"
