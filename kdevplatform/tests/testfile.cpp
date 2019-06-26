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
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <project/projectmodel.h>

using namespace KDevelop;

class KDevelop::TestFilePrivate
{
public:
    TestFilePrivate()
    {
    }

    void updateReady(const IndexedString& _url, const ReferencedTopDUContext& _topContext)
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
    bool ready = false;
    ReferencedTopDUContext topContext;
    IndexedString url;
    TestProject* project;
    QScopedPointer<ProjectFileItem> fileItem;
    bool keepDUChainData = false;
};

TestFile::TestFile(const QString& contents, const QString& fileExtension,
                   TestProject* project, const QString& dir)
    : d_ptr(new TestFilePrivate())
{
    Q_D(TestFile);

    d->suffix = QLatin1Char('.') + fileExtension;

    QTemporaryFile file((!dir.isEmpty() ? dir : QDir::tempPath()) + QLatin1String("/testfile_XXXXXX") + d->suffix);
    file.setAutoRemove(false);
    file.open();
    Q_ASSERT(file.isOpen());

    d->init(file.fileName(), contents, project);
}

TestFile::TestFile(const QString& contents, const QString& fileExtension, const TestFile* base)
    : d_ptr(new TestFilePrivate)
{
    Q_D(TestFile);

    QString fileName = base->d_func()->file.mid(0, base->d_func()->file.length() - base->d_func()->suffix.length());
    d->suffix = QLatin1Char('.') + fileExtension;
    fileName += d->suffix;
    d->init(fileName, contents, base->d_func()->project);
}

TestFile::TestFile(const QString& contents, const QString& fileExtension, const QString& fileName,
                   KDevelop::TestProject* project, const QString& dir)
    : d_ptr(new TestFilePrivate)
{
    Q_D(TestFile);

    d->suffix = QLatin1Char('.') + fileExtension;
    const QString file = (!dir.isEmpty() ? dir : QDir::tempPath())
                    + QLatin1Char('/') + fileName + d->suffix;
    d->init(file, contents, project);
}


TestFile::~TestFile()
{
    Q_D(TestFile);

    if (auto* document = ICore::self()->documentController()->documentForUrl(d->url.toUrl())) {
        document->close(KDevelop::IDocument::Discard);
    }

    auto backgroundParser = ICore::self()->languageController()->backgroundParser();
    backgroundParser->removeDocument(d->url, this);
    QTRY_VERIFY(!backgroundParser->parseJobForDocument(d->url));

    if (d->topContext && !d->keepDUChainData) {
        DUChainWriteLocker lock;
        DUChain::self()->removeDocumentChain(d->topContext.data());
    }
    QFile::remove(d->file);
}

IndexedString TestFile::url() const
{
    Q_D(const TestFile);

    return d->url;
}

void TestFile::parse(TopDUContext::Features features, int priority)
{
    Q_D(TestFile);

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
    Q_D(TestFile);

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
    Q_D(const TestFile);

    return d->ready;
}

ReferencedTopDUContext TestFile::topContext()
{
    Q_D(TestFile);

    waitForParsed();
    return d->topContext;
}

void TestFile::setFileContents(const QString& contents)
{
    Q_D(TestFile);

    d->setFileContents(contents);
}

QString TestFile::fileContents() const
{
    Q_D(const TestFile);

    QFile file(d->file);
    file.open(QIODevice::ReadOnly);
    Q_ASSERT(file.isOpen());
    Q_ASSERT(file.isReadable());
    return QString::fromUtf8(file.readAll());
}

void TestFile::setKeepDUChainData(bool keep)
{
    Q_D(TestFile);

    d->keepDUChainData = keep;
}

bool TestFile::keepDUChainData() const
{
    Q_D(const TestFile);

    return d->keepDUChainData;
}

#include "moc_testfile.cpp"
