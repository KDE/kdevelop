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

using namespace KDevelop;

struct TestFile::TestFilePrivate {
    TestFilePrivate(const QString& fileExtension)
    : file("XXXXXX." + fileExtension)
    , ready(false)
    {
    }

    void updateReady(const IndexedString& _url, ReferencedTopDUContext _topContext)
    {
        Q_ASSERT(_url == url);
        topContext = _topContext;
        ready = true;
    }

    QTemporaryFile file;
    bool ready;
    ReferencedTopDUContext topContext;
    IndexedString url;
};

TestFile::TestFile (const QString& contents, const QString& fileExtension, TestProject* project)
: d(new TestFilePrivate(fileExtension))
{
    d->file.open();
    QVERIFY(d->file.isOpen());
    QVERIFY(d->file.isWritable());
    d->file.write(contents.toLocal8Bit());
    d->file.close();

    d->url = IndexedString(d->file.fileName());

    project->addToFileSet(d->url);
}

TestFile::~TestFile()
{
    if (d->topContext) {
        DUChainWriteLocker lock;
        DUChain::self()->removeDocumentChain(d->topContext.data());
    }
}

void TestFile::parse (TopDUContext::Features features, int priority)
{
    d->ready = false;
    DUChain::self()->updateContextForUrl(d->url, features, this, priority);
}

bool TestFile::waitForParsed(int timeout)
{
    QTime t;
    t.start();
    while (!d->ready && t.elapsed() < timeout) {
        QTest::qWait(10);
    }
    return d->ready;
}

ReferencedTopDUContext TestFile::topContext()
{
    waitForParsed();
    return d->topContext;
}

#include "testfile.moc"
