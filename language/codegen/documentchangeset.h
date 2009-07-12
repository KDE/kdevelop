/*
   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef DOCUMENTCHANGESET_H
#define DOCUMENTCHANGESET_H

#include <language/editor/simplerange.h>
#include <language/duchain/indexedstring.h>
#include <ksharedptr.h>

namespace KDevelop {
    
struct DocumentChangeSetPrivate;

struct KDEVPLATFORMLANGUAGE_EXPORT DocumentChange : public QSharedData {
    DocumentChange(const IndexedString& document, const SimpleRange& range, const QString& oldText, const QString& newText) : m_document(document), m_range(range), m_oldText(oldText), m_newText(newText), m_ignoreOldText(false) {
    }
    
    IndexedString m_document;
    SimpleRange m_range;
    QString m_oldText;
    QString m_newText;
    bool m_ignoreOldText; //Set this to disable the verification of m_oldText. This can be used to overwrite arbitrary text, but is dangerous!
};

typedef KSharedPtr<DocumentChange> DocumentChangePointer;

class KDEVPLATFORMLANGUAGE_EXPORT DocumentChangeSet {
  public:
    
      DocumentChangeSet();
      ~DocumentChangeSet();
    
    //Returns true on success
    struct ChangeResult {
        ChangeResult(bool success) : m_success(success) {
        }
        ChangeResult(QString failureReason, DocumentChangePointer reasonChange = DocumentChangePointer()) : m_failureReason(failureReason), m_reasonChange(reasonChange) {
        }
        
        operator bool() const {
            return m_success;
        }
        
        QString m_failureReason;
        DocumentChangePointer m_reasonChange;
        
        bool m_success;
    };
    
    ///@deprecated
    ChangeResult addChange(const DocumentChangePointer& change);
    ///If the change has multiple lines, a problem will be returned. these don't work at he moment.
    ChangeResult addChange(const DocumentChange& change);
    ///Get rid of all the current changes
    void clear(void);
    
    ///Merge two document change sets
    DocumentChangeSet & operator<<(DocumentChangeSet &);
    
    enum ReplacementPolicy {
        IgnoreFailedChange,///If this is given, all changes that could not be applied are simply ignored
        WarnOnFailedChange,///If this is given to applyAllChanges, a warning is given when a change could not be applied,
                            ///but following changes are applied, and success is returned.
        StopOnFailedChange ///If this is given to applyAllChanges, then all replacements are reverted and an error returned on problems
    };
    ///@param policy What should be done when a change could not be applied?
    void setReplacementPolicy(ReplacementPolicy policy);
    
    enum FormatPolicy {
        NoAutoFormat, ///If this option is given, no automatic formatting is applied
        AutoFormatChanges      ///If this option is given, all changes are automatically reformatted using the formatter plugin for the mime type
    };
    ///@param policy How the changed text should be formatted
    void setFormatPolicy(FormatPolicy policy);
    
    enum DUChainUpdateHandling {
        NoUpdate,       ///No updates will be scheduled
        SimpleUpdate ///The changed documents will be added to the background parser, plus all documents that are currently open and recursively import those documetns
        //FullUpdate       ///All documents in all open projects that recursively import any of the changed documents will be updated
    };
    ///@param policy Whether a duchain update should be triggered for all affected documents
    void setUpdateHandling(DUChainUpdateHandling policy);
    
    ///Apply all the registered changes for @p file, and keep them as temporary in memory
    ChangeResult applyToTemp(IndexedString file);
    
    ///Apply all changes for all files, and keep them as temporary in memory
    ChangeResult applyAllToTemp();
    
    /// @return The name for the latest temporary version of @p file if it exists, returns @p file otherwise
    IndexedString tempNameForFile(IndexedString file);
    
    /// Apply all the changes registered in this changeset
    ChangeResult applyAllChanges();
    
  private:
        
    DocumentChangeSetPrivate * d;
};
}

#endif
