/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_CODEREPRESENTATION_H
#define KDEVPLATFORM_CODEREPRESENTATION_H

#include <language/languageexport.h>
#include <serialization/indexedstring.h>

#include <KTextEditor/Document>

#include <memory>

class QString;

namespace KTextEditor {
class Range;
}

namespace KDevelop {
struct KDevEditingTransaction;

class IndexedString;

//NOTE: this is ugly, but otherwise kate might remove tabs again :-/
// see also: https://bugs.kde.org/show_bug.cgi?id=291074
struct EditorDisableReplaceTabs
{
    explicit EditorDisableReplaceTabs(KTextEditor::Document* document)
        : m_document(document)
        , m_count(0)
    {
        Q_ASSERT(m_document);

        ++m_count;
        if (m_count > 1)
            return;

        m_oldReplaceTabs = m_document->configValue(configKey());
        m_document->setConfigValue(configKey(), false);
    }

    ~EditorDisableReplaceTabs()
    {
        --m_count;
        if (m_count > 0)
            return;

        Q_ASSERT(m_count == 0);

        m_document->setConfigValue(configKey(), m_oldReplaceTabs);
    }

    inline QString configKey() const
    {
        return QStringLiteral("replace-tabs");
    }

private:
    Q_DISABLE_COPY(EditorDisableReplaceTabs)

    KTextEditor::Document* const m_document;
    int m_count;
    QVariant m_oldReplaceTabs;
};

struct KDevEditingTransaction
{
    explicit KDevEditingTransaction(KTextEditor::Document* document)
        : edit(document)
        , disableReplaceTabs(document)
    {}
    // NOTE: It's important to close the transaction first and only then destroy the EditorDisableReplaceTabs. Otherwise we hit asserts in KTextEditor.
    KTextEditor::Document::EditingTransaction edit;
    EditorDisableReplaceTabs disableReplaceTabs;
    using Ptr = std::unique_ptr<KDevEditingTransaction>;
};

/**
 * Allows getting code-lines conveniently, either through an open editor, or from a disk-loaded file.
 */
class KDEVPLATFORMLANGUAGE_EXPORT CodeRepresentation
    : public QSharedData
{
public:
    virtual ~CodeRepresentation()
    {
    }
    virtual QString line(int line) const = 0;
    virtual int lines() const = 0;
    virtual QString text() const = 0;
    virtual QString rangeText(const KTextEditor::Range& range) const;
    /**
     * Search for the given identifier in the document, and returns all ranges
     * where it was found.
     * @param identifier The identifier to search for
     * @param surroundedByBoundary Whether only matches that are surrounded by typical word-boundaries
     *                             should be acceded. Everything except letters, numbers, and the _ character
     *                             counts as word boundary.
     * */
    virtual QVector<KTextEditor::Range> grep(const QString& identifier, bool surroundedByBoundary = true) const = 0;
    /**
     * Overwrites the text in the file with the new given one
     *
     * @return true on success
     */
    virtual bool setText(const QString&) = 0;
    /** @return true if this representation represents an actual file on disk */
    virtual bool fileExists() const = 0;

    /**
     * Can be used for example from tests to disallow on-disk changes. When such a change is done, an assertion is triggered.
     * You should enable this within tests, unless you really want to work on the disk.
     */
    static void setDiskChangesForbidden(bool changesForbidden);

    /**
     * Returns the specified name as a url for artificial source code
     * suitable for code being inserted into the parser
     */
    static QString artificialPath(const QString& name);

    using Ptr = QExplicitlySharedDataPointer<CodeRepresentation>;
};

class KDEVPLATFORMLANGUAGE_EXPORT DynamicCodeRepresentation
    : public CodeRepresentation
{
public:
    /** Used to group edit-history together. Call this optionally before a bunch
     *  of replace() calls, to group them together. */
    virtual KDevEditingTransaction::Ptr makeEditTransaction() = 0;
    virtual bool replace(const KTextEditor::Range& range, const QString& oldText,
                         const QString& newText, bool ignoreOldText = false) = 0;

    using Ptr = QExplicitlySharedDataPointer<DynamicCodeRepresentation>;
};

/**
 * Creates a code-representation for the given url, that allows conveniently accessing its data. Returns zero on failure.
 */
KDEVPLATFORMLANGUAGE_EXPORT CodeRepresentation::Ptr createCodeRepresentation(const IndexedString& url);

/**
 * @return true if an artificial code representation already exists for the specified URL
 */
KDEVPLATFORMLANGUAGE_EXPORT bool artificialCodeRepresentationExists(const IndexedString& url);

/**
 *   Allows inserting artificial source-code into the code-representation and parsing framework.
 *  The source-code logically represents a file.
 *
 *  The artificial code is automatically removed when the object is destroyed.
 */
class KDEVPLATFORMLANGUAGE_EXPORT InsertArtificialCodeRepresentation
    : public QSharedData
{
public:
    /**
     * Inserts an artificial source-code representation with filename @p file and the contents @p text
     * If @p file is not an absolute path or url, it will be made absolute using the CodeRepresentation::artificialUrl()
     * function, while ensuring that the name is unique.
     */
    InsertArtificialCodeRepresentation(const IndexedString& file, const QString& text);
    ~InsertArtificialCodeRepresentation();
    InsertArtificialCodeRepresentation(const InsertArtificialCodeRepresentation&) = delete;
    InsertArtificialCodeRepresentation& operator=(const InsertArtificialCodeRepresentation&) = delete;

    void setText(const QString& text);
    QString text() const;
    /**
     * Returns the file-name for this code-representation.
     */
    IndexedString file();

private:
    IndexedString m_file;
};

using InsertArtificialCodeRepresentationPointer = QExplicitlySharedDataPointer<InsertArtificialCodeRepresentation>;
}

#endif
