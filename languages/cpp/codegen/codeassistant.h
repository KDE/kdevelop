/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
   
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

#ifndef CPP_CODEASSISTANT_H
#define CPP_CODEASSISTANT_H

#include <interfaces/iassistant.h>
#include <QObject>
#include <ktexteditor/cursor.h>
#include <language/duchain/declarationid.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/indexedstring.h>
#include <language/duchain/types/indexedtype.h>
#include <QTimer>
#include "renameassistant.h"

typedef QWeakPointer<KTextEditor::Document> SafeDocumentPointer;

namespace KTextEditor {
class Document;
class Range;
class View;
}
namespace KDevelop {
class IDocument;
class ParseJob;
class DUContext;
}
namespace Cpp {

class StaticCodeAssistant : public QObject {
  Q_OBJECT
  public:
    StaticCodeAssistant();
    
  private slots:
    void assistantHide();
    void documentLoaded(KDevelop::IDocument*);
    void textInserted(KTextEditor::Document*,KTextEditor::Range);
    void textRemoved(KTextEditor::Document*,KTextEditor::Range,const QString& removedText);
    void parseJobFinished(KDevelop::ParseJob*);
    void documentActivated(KDevelop::IDocument*);
    void cursorPositionChanged(KTextEditor::View*,KTextEditor::Cursor);
    void timeout();
    void eventuallyStartAssistant();
    void deleteRenameAssistantsForDocument(KTextEditor::Document*);
  private:
    void checkAssistantForProblems(KDevelop::TopDUContext* top);
    void startAssistant(KSharedPtr< KDevelop::IAssistant > assistant);
    QWeakPointer<KTextEditor::View> m_currentView;
    KTextEditor::Cursor m_assistantStartedAt;
    KDevelop::IndexedString m_currentDocument;
    KSharedPtr<KDevelop::IAssistant> m_activeAssistant;
    QHash< KTextEditor::View*, KSharedPtr<RenameAssistant> > m_renameAssistants;
    bool m_activeProblemAssistant;
    QTimer* m_timer;

    SafeDocumentPointer m_eventualDocument;
    KTextEditor::Range m_eventualRange;
    QString m_eventualRemovedText;
    //Singleton pointer
    static StaticCodeAssistant * instance;
};

}

#endif // CPP_CODEASSISTANT_H
