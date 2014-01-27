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
     * @param fileExtension the file extension without the dot.
     * @param project this file will be added to the project's fileset and gets
     *                removed from there on destruction
     * @param dir optional path to a (sub-) directory in which this file should
     *            be created. The directory must exist.
     *
     * Example:
     * @code
     * TestFile file("int i = 0;", "cpp");
     * file.parse(...);
     * file.topContext()->...
     * @endcode
     */
    TestFile(const QString& contents, const QString& fileExtension, KDevelop::TestProject *project = 0,
             const QString& dir = QString());

    /**
     * Create a temporary file from @p contents with the same file basename as
     * @p other but with the given @p fileExtension.
     *
     * @param fileExtension the new file extension without the dot.
     * @param other a different TestFile which is used for this file's basename
     *
     * This can be used to create e.g. .cpp/.h file pairs:
     *
     * @code
     * TestFile header("...", "h");
     * TestFile impl("...", "cpp", &header);
     * @endcode
     */
    TestFile(const QString& contents, const QString& fileExtension, const TestFile* base);

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
    void parse(TopDUContext::Features features = TopDUContext::AllDeclarationsContextsAndUses, int priority = 1);

    /**
     * Convenience method:
     * Trigger parse and wait for the file to be parsed. Internally calls waitForParsed()
     *
     * @see waitForParsed()
     * @see parse()
     */
    bool parseAndWait(TopDUContext::Features features = TopDUContext::AllDeclarationsContextsAndUses,
                      int priority = 1, int timeout = 1000);

    /**
     * Blocks current thread and waits until the file has been parsed.
     *
     * If it has waited longer than @p timeout ms, we return false
     * and assume something went wrong.
     *
     * Otherwise true is returned, indicating parsing finished
     * within the timeout interval.
     */
    bool waitForParsed(int timeout = 1000);

    /**
     * Check whether the file has been processed after the last call to @c parse().
     */
    bool isReady() const;

    /**
     * Returns the @c TopDUContext for the current file, if it has been successfully parsed.
     */
    KDevelop::ReferencedTopDUContext topContext();

    /**
     * Change the file contents to @p contents.
     *
     * Use this to test behavior of your parsing code over
     * file changes.
     */
    void setFileContents(const QString& contents);

    /**
     * Read the files contents and return them.
     */
    QString fileContents() const;

private:
    struct TestFilePrivate;
    TestFilePrivate* d;

    Q_PRIVATE_SLOT(d, void updateReady(const KDevelop::IndexedString& url, KDevelop::ReferencedTopDUContext topContext))
};

}

#endif // KDEVPLATFORM_TESTFILE_H
