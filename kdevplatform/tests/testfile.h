/*
    SPDX-FileCopyrightText: 2010 Niko Sams <niko.sams@gmail.com>
    SPDX-FileCopyrightText: 2011 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_TESTFILE_H
#define KDEVPLATFORM_TESTFILE_H

#include <QObject>

#include <language/duchain/topducontext.h>

#include "testsexport.h"

namespace KDevelop {
class TestProject;
class TestFilePrivate;

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
class KDEVPLATFORMTESTS_EXPORT TestFile
    : public QObject
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
    TestFile(const QString& contents, const QString& fileExtension, KDevelop::TestProject* project = nullptr,
             const QString& dir = QString());

    /**
     * Create a temporary file from @p contents with the same file basename as
     * @p base but with the given @p fileExtension.
     *
     * @param fileExtension the new file extension without the dot.
     * @param base a different TestFile which is used for this file's basename
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
     * Create a temporary file named @p fileName from @p contents with file extension @p extension.
     *
     * @param fileExtension the file extension without the dot.
     * @param fileName the name to use for the file
     * @param project this file will be added to the project's fileset and gets
     *                removed from there on destruction
     * @param dir optional path to a (sub-) directory in which this file should
     *            be created. The directory must exist.
     *
     * Example:
     * @code
     * TestFile file("int i = 0;", "h", "guard_test");
     * @endcode
     */    TestFile(const QString& contents, const QString& fileExtension, const QString& fileName,
             KDevelop::TestProject* project = nullptr, const QString& dir = QString());

    /**
     * Removes temporary file and cleans up.
     */
    ~TestFile() override;

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
                      int priority = 1, int timeout = 30'000);

    /**
     * Blocks current thread and waits until the file has been parsed.
     *
     * If it has waited longer than @p timeout ms, we return false
     * and assume something went wrong.
     *
     * Otherwise true is returned, indicating parsing finished
     * within the timeout interval.
     */
    bool waitForParsed(int timeout = 30'000);

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

    /**
     * Set to true when you want to keep the DUChain data.
     *
     * By default the DUChain data is removed on destruction of the TestFile.
     */
    void setKeepDUChainData(bool keep);
    bool keepDUChainData() const;

private:
    const QScopedPointer<class TestFilePrivate> d_ptr;
    Q_DECLARE_PRIVATE(TestFile)

    Q_PRIVATE_SLOT(d_func(), void updateReady(const KDevelop::IndexedString& url, KDevelop::ReferencedTopDUContext topContext))
};
}

#endif // KDEVPLATFORM_TESTFILE_H
