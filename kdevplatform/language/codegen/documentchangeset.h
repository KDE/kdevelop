/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_DOCUMENTCHANGESET_H
#define KDEVPLATFORM_DOCUMENTCHANGESET_H

#include <language/languageexport.h>

#include <serialization/indexedstring.h>

#include <KTextEditor/Range>

#include <QExplicitlySharedDataPointer>
#include <QUrl>

namespace KDevelop {
class DocumentChangeSetPrivate;

class KDEVPLATFORMLANGUAGE_EXPORT DocumentChange
    : public QSharedData
{
public:

    DocumentChange(const IndexedString& document, const KTextEditor::Range& range, const QString& oldText,
                   const QString& newText) :
        m_document(document)
        , m_range(range)
        , m_oldText(oldText)
        , m_newText(newText)
        , m_ignoreOldText(false)
    {
        //Clean the URL, so we don't get the same file be stored as a different one
        QUrl url = m_document.toUrl();
        m_document = IndexedString(url.adjusted(QUrl::NormalizePathSegments));
    }

    IndexedString m_document;
    KTextEditor::Range m_range;
    QString m_oldText;
    QString m_newText;
    bool m_ignoreOldText; //Set this to disable the verification of m_oldText. This can be used to overwrite arbitrary text, but is dangerous!
};

using DocumentChangePointer = QExplicitlySharedDataPointer<DocumentChange>;

/**
 * Object representing an arbitrary set of changes to an arbitrary set of files that can be applied atomically.
 */
class KDEVPLATFORMLANGUAGE_EXPORT DocumentChangeSet
{
public:
    DocumentChangeSet();
    ~DocumentChangeSet();
    DocumentChangeSet(const DocumentChangeSet& rhs);
    DocumentChangeSet& operator=(const DocumentChangeSet& rhs);

    //Returns true on success
    class ChangeResult
    {
public:
        explicit ChangeResult(const QString& failureReason,
                              const DocumentChangePointer& reasonChange = DocumentChangePointer())
            : ChangeResult(failureReason, reasonChange, false) {}

        static ChangeResult successfulResult()
        {
            return ChangeResult({}, {}, true);
        }

        operator bool() const
        {
            return m_success;
        }

        /// Reason why the change failed
        QString m_failureReason;
        /// Specific change that caused the problem (might be 0)
        DocumentChangePointer m_reasonChange;

        bool m_success;

private:
        explicit ChangeResult(const QString& failureReason, const DocumentChangePointer& reasonChange, bool success)
            : m_failureReason(failureReason)
            , m_reasonChange(reasonChange)
            , m_success(success)
        {}
    };

    /// Add an individual local change to this change-set.
    ChangeResult addChange(const DocumentChange& change);
    ChangeResult addChange(const DocumentChangePointer& change);

    ///given a file @p oldFile, rename it to the @p newname
    ChangeResult addDocumentRenameChange(const IndexedString& oldFile, const IndexedString& newname);

    enum ReplacementPolicy {
        IgnoreFailedChange,///If this is given, all changes that could not be applied are simply ignored
        WarnOnFailedChange,///If this is given to applyAllChanges, a warning is given when a change could not be applied,
        ///but following changes are applied, and success is returned.
        StopOnFailedChange ///If this is given to applyAllChanges, then all replacements are reverted and an error returned on problems (default)
    };

    ///@param policy What should be done when a change could not be applied?
    void setReplacementPolicy(ReplacementPolicy policy);

    enum FormatPolicy {
        NoAutoFormat, ///If this option is given, no automatic formatting is applied
        AutoFormatChanges,      ///If this option is given, all changes are automatically reformatted using the formatter plugin for the mime type (default)
        AutoFormatChangesKeepIndentation      ///Same as AutoFormatChanges, except that the indentation of inserted lines is kept equal
    };

    ///@param policy How the changed text should be formatted. The default is AutoFormatChanges.
    void setFormatPolicy(FormatPolicy policy);

    enum DUChainUpdateHandling {
        NoUpdate,       ///No updates will be scheduled
        SimpleUpdate ///The changed documents will be added to the background parser, plus all documents that are currently open and recursively import those documents (default)
        //FullUpdate       ///All documents in all open projects that recursively import any of the changed documents will be updated
    };

    ///@param policy Whether a duchain update should be triggered for all affected documents
    void setUpdateHandling(DUChainUpdateHandling policy);

    enum ActivationPolicy {
        Activate,           ///The affected files will be activated
        DoNotActivate  ///The affected files will not be activated (default)
    };

    ///@param policy Whether the affected documents should be activated when the change is applied
    void setActivationPolicy(ActivationPolicy policy);

    /// Apply all the changes registered in this changeset to the actual files
    ChangeResult applyAllChanges();

private:
    const QScopedPointer<class DocumentChangeSetPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DocumentChangeSet)
};
}

#endif
