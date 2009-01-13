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
    struct KDEVPLATFORMLANGUAGE_EXPORT DocumentChange : public KShared {
        DocumentChange(const IndexedString& document, const SimpleRange& range, const QString& oldText, const QString& newText) : m_document(document), m_range(range), m_oldText(oldText), m_newText(newText) {
        }
        
        IndexedString m_document;
        SimpleRange m_range;
        QString m_oldText;
        QString m_newText;
    };
    
    typedef KSharedPtr<DocumentChange> DocumentChangePointer;
    
    class KDEVPLATFORMLANGUAGE_EXPORT DocumentChangeSet {
        public:
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
        
        ///If the change has multiple lines, a problem will be returned. these don't work at he moment.
        ChangeResult addChange(const DocumentChangePointer& change);
        
        enum ReplacementPolicy {
            IgnoreFailedChange,///If this is given, all changes that could not be applied are simply ignored
            WarnOnFailedChange,///If this is given to applyAllChanges, a warning is given when a change could not be applied,
                               ///but following changes are applied, and success is returned.
            StopOnFailedChange ///If this is given to applyAllChanges, then all replacements are reverted and an error returned on problems
        };
        
        ///@param policy What should be done when a change could not be applied?
        ChangeResult applyAllChanges(ReplacementPolicy policy = StopOnFailedChange);
        
        private:
            QMap< IndexedString, QList<DocumentChangePointer> > m_changes;
    };
}

#endif
