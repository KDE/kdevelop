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

#ifndef KDEVPLATFORM_TESTFILE_H
#define KDEVPLATFORM_TESTFILE_H

#include <QObject>

#include <language/duchain/topducontext.h>

#include "kdevplatformtestsexport.h"

namespace KDevelop {

class TestProject;

/**
 * Helper file to parse a file using the full KDevelop architecture.
 * 
 * The file will be added to the background parser, and eventually
 * parsed by a fitting language plugin, just like a normal file
 * would be in an actual KDevelop instance.
 *
 * Example usage:
 * \code
 * TestFile file("<?php function foo(){}", "php", project);
 * file.parse(TopDUContext::VisibleDeclarationsAndContexts);
 * QVERIFY(file.waitForParsed());
 * QVERIFY(file.topContext());
 * \endcode
 */
class KDEVPLATFORMTESTS_EXPORT TestFile : public QObject
{
    Q_OBJECT

public:
    /**
     * Create a temporary file from @p contents with file extension @p extension.
     *
     * It is added to @p project's fileset.
     */
    TestFile(const QString& contents, const QString& fileExtenion, KDevelop::TestProject *project);

    /**
     * Removes temporary file and cleans up.
     */
    ~TestFile();

    /**
     * Returns the URL to this file.
     */
    IndexedString url() const;

    /**
     * Trigger (re-)parsing of this file with given @p features and @p priority.
     * 
     * @see KDevelop::DUChain::updateContextForUrl
     */
    void parse(KDevelop::TopDUContext::Features features, int priority = 1);

    /**
     * Blocks current thread and waits until the file has been parsed.
     *
     * If it has waited longer than @p timeout ms, we return false
     * and assume something went wrong.
     *
     * Otherwise true is returned, indicating parsing finished
     * within the timeout interval.
     */
    bool waitForParsed(int timeout = 60000);

    /**
     * Returns the @c TopDUContext for the current file, if it has been successfully parsed.
     */
    KDevelop::ReferencedTopDUContext topContext();

private:
    struct TestFilePrivate;
    TestFilePrivate* d;

    Q_PRIVATE_SLOT(d, void updateReady(const KDevelop::IndexedString& url, KDevelop::ReferencedTopDUContext topContext))
};

}

#endif // KDEVPLATFORM_TESTFILE_H
